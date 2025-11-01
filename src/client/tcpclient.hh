#ifndef NEBLINA_TCP_CLIENT_HH
#define NEBLINA_TCP_CLIENT_HH

#include "client.hh"
#include "util/socket.hh"

class TCPClient : public Client {
public:
    TCPClient(std::string const& host, uint16_t port);

    void send(std::string const& data) const override;

protected:
    [[nodiscard]] std::string recv(size_t n_bytes) const override;

private:
    [[nodiscard]] SOCKET open_connection(std::string const& host, uint16_t port) const;
};


#endif //NEBLINA_TCP_CLIENT_HH
