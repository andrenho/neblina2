#ifndef NEBLINA_SSLCLIENT_HH
#define NEBLINA_SSLCLIENT_HH

#include <openssl/ssl.h>

#include "tcpclient.hh"

class SSLClient : public TCPClient {
public:
    SSLClient(std::string const& host, uint16_t port);
    ~SSLClient() override;

    void send(std::string const &data) const override;

protected:
    [[nodiscard]] std::string recv(size_t n_bytes) const override;

    SSL* ssl_ = nullptr;
    static std::unique_ptr<SSL_CTX, void(*)(SSL_CTX*)> ctx_;
};


#endif //NEBLINA_SSLCLIENT_HH
