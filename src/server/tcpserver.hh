#ifndef NEBLINA_TCP_SERVER_HH
#define NEBLINA_TCP_SERVER_HH

#include <cstdint>
#include <string>

#include "server.hh"

class TCPServer : public Server {
public:
    TCPServer(uint16_t port, bool open_to_world, std::unique_ptr<Protocol> protocol, size_t n_threads);

protected:
    [[nodiscard]] std::unique_ptr<Socket> accept_new_connection() const override;

    std::string recv(SOCKET fd) const override;
    void send(SOCKET fd, const std::string &data) const override;

private:
    [[nodiscard]] static SOCKET create_listener(uint16_t port, bool open_to_world);
};


#endif //NEBLINA_TCP_SERVER_HH
