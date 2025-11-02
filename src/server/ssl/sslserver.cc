#include "sslserver.hh"

#include <openssl/err.h>

#include "sslconnection.hh"
#include "util/log.hh"
#include "util/exceptions/non_recoverable_exception.hh"

using namespace std::string_literals;

static int err(char const* str, size_t len, void*)
{
    ERR("ssl server: {}", str);
    throw NonRecoverableException("ssl server: {}"s + str);
}

SSLServer::SSLServer(uint16_t port, bool open_to_world, std::unique_ptr<Protocol> protocol, ThreadCount thread_count,
                     std::string const &certificate, std::string const &private_key)
    : TCPServer(port, open_to_world, std::move(protocol), thread_count)
{
    SSL_METHOD const* method = TLS_server_method();
    ctx_ = SSL_CTX_new(method);

    // Load cert from string
    BIO *certbio = BIO_new_mem_buf(certificate.c_str(), -1);
    if (!certbio)
        throw NonRecoverableException("Invalid certificate key.");
    X509 *cert = PEM_read_bio_X509(certbio, nullptr, nullptr, nullptr);
    if (!cert)
        throw NonRecoverableException("Invalid certificate key.");
    SSL_CTX_use_certificate(ctx_, cert);
    X509_free(cert);
    BIO_free(certbio);

    // Load key from string
    BIO *keybio = BIO_new_mem_buf(private_key.c_str(), -1);
    if (!keybio)
        ERR_print_errors_cb(err, nullptr);
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(keybio, nullptr, nullptr, nullptr);
    if (!pkey)
        ERR_print_errors_cb(err, nullptr);
    SSL_CTX_use_PrivateKey(ctx_, pkey);
    EVP_PKEY_free(pkey);
    BIO_free(keybio);

    if (!SSL_CTX_check_private_key(ctx_))
        ERR_print_errors_cb(err, nullptr);
}

SSLServer::~SSLServer()
{
    if (ctx_)
        SSL_CTX_free(ctx_);
}

std::unique_ptr<Connection> SSLServer::create_connection(SOCKET fd_) const
{
    return std::make_unique<SSLConnection>(fd_, ctx_);
}