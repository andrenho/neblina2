#ifndef NEBLINA_TCPSERVER_HH
#define NEBLINA_TCPSERVER_HH

#include "server/server.hh"

class TCPServer : public Server {
public:
    TCPServer(uint16_t port, bool open_to_world, std::unique_ptr<Protocol> protocol, ThreadCount thread_count)
        : Server(create_listener(port, open_to_world), std::move(protocol), thread_count) {}

protected:
    [[nodiscard]] std::unique_ptr<Connection> create_connection(SOCKET fd_) const override;
    [[nodiscard]] SOCKET                      accept() const override;

private:
    [[nodiscard]] SOCKET create_listener(uint16_t port, bool open_to_world) const;
};

#endif //NEBLINA_TCPSERVER_HH
