#ifndef NEBLINA_SSLSERVER_HH
#define NEBLINA_SSLSERVER_HH

#include <openssl/ssl.h>

#include "../tcp/tcpserver.hh"

class SSLServer : public TCPServer {
public:
    SSLServer(uint16_t port, bool open_to_world, std::unique_ptr<Protocol> protocol, ThreadCount thread_count,
              std::string const& certificate, std::string const& private_key);
    ~SSLServer();

protected:
    [[nodiscard]] std::unique_ptr<Connection> create_connection(SOCKET fd_) const override;

private:
    SSL_CTX *ctx_ = nullptr;
};


#endif //NEBLINA_SSLSERVER_HH
