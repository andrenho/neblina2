#ifndef NEBLINA_SSLCONNECTION_HH
#define NEBLINA_SSLCONNECTION_HH

#include <openssl/ssl.h>

#include "../connection.hh"

class SSLConnection : public Connection {
public:
    explicit SSLConnection(SOCKET fd, SSL_CTX* ctx);
    ~SSLConnection();

    [[nodiscard]] std::string recv() const override;
    void                      send(std::string const &data) const override;

private:
    SSL* ssl_ = nullptr;
};


#endif //NEBLINA_SSLCONNECTION_HH
