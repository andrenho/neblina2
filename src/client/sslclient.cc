#include "sslclient.hh"

#include <openssl/err.h>

#include "util/exceptions/non_recoverable_exception.hh"

using namespace std::string_literals;

decltype(SSLClient::ctx_) SSLClient::ctx_ { nullptr, [](SSL_CTX*){} };

static int err(char const* str, size_t len, void*)
{
    ERR("ssl client: {}", str);
    throw NonRecoverableException("ssl server: {}"s + str);
}

SSLClient::SSLClient(std::string const &host, uint16_t port)
    : TCPClient(host, port)
{
    initialize_ctx();

    ssl_ = SSL_new(ctx_.get());
    SSL_set_tlsext_host_name(ssl_, host.c_str());
    SSL_set_fd(ssl_, fd_);

try_again:
    int r = SSL_connect(ssl_);
    if (r < 0) {
        int err = SSL_get_error(ssl_, r);
        if (err == SSL_ERROR_WANT_READ)
            goto try_again;
    }
}

SSLClient::~SSLClient()
{
    if (ssl_)
        SSL_free(ssl_);
}

void SSLClient::initialize_ctx()
{
    static std::mutex mutex;

    std::lock_guard lock(mutex);
    if (!ctx_) {
        ctx_ = { SSL_CTX_new(TLS_client_method()), [](SSL_CTX* ctx) { SSL_CTX_free(ctx); } };
        if (!SSL_CTX_set_default_verify_paths(ctx_.get()))
            throw NonRecoverableException("Failed to load system CA store");
    }
}

void SSLClient::send(std::string const &data) const
{
    size_t pos = 0;
    while (pos < data.size()) {
        ssize_t r = SSL_write(ssl_, data.data(), (int) data.size());
        if (r < 0)
            throw std::runtime_error("send error: "s + strerror(errno));
        pos += r;
    }
}

std::string SSLClient::recv(size_t n_bytes) const
{
    std::string buf(n_bytes, 0);
    ssize_t r = SSL_read(ssl_, buf.data(), (int) n_bytes);
#ifdef _WIN32
    if (r == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            return "";
        }
        else {
            throw std::runtime_error("recv error: " + std::to_string(err));
        }
    }
#endif
    if (r < 0) {
        if (errno == EAGAIN)
            return "";
        else
            throw std::runtime_error("recv error: "s + strerror(errno));
    } else if (r == 0) {
        return "";
    } else {
        buf.resize(r);
        return buf;
    }
}

