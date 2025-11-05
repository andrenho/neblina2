#include "sslconnection.hh"

#include <openssl/err.h>

using namespace std::string_literals;

static int err(char const* str, size_t len, void*)
{
    ERR("ssl server connection: {}", str);
    throw std::runtime_error("ssl server: {}"s + str);
}

SSLConnection::SSLConnection(SOCKET fd, SSL_CTX* ctx)
    : Connection(fd)
{
    ssl_ = SSL_new(ctx);
    SSL_set_fd(ssl_, fd);

    if (SSL_accept(ssl_) <= 0)
        ERR_print_errors_cb(err, nullptr);
}

SSLConnection::~SSLConnection()
{
    SSL_shutdown(ssl_);
    SSL_free(ssl_);
}

std::string SSLConnection::recv() const
{
    static constexpr size_t RECV_BUF_SZ = 16 * 1024;
    std::string buf(RECV_BUF_SZ, 0);
    ssize_t r = SSL_read(ssl_, buf.data(), RECV_BUF_SZ);
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

void SSLConnection::send(std::string const &data) const
{
    size_t pos = 0;
    while (pos < data.size()) {
        ssize_t r = SSL_write(ssl_, data.data(), (int) data.size());
        if (r < 0)
            throw std::runtime_error("send error: "s + strerror(errno));
        pos += r;
    }
}

