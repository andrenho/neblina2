#ifndef NEBLINA_SERVER_HH
#define NEBLINA_SERVER_HH

#include "server/poller/poller.hh"
#include "protocol/protocol.hh"
#include "connection.hh"

#include <memory>

class Server {
public:
    virtual ~Server() { close_socket(fd_); }

protected:
    explicit Server(SOCKET fd, std::unique_ptr<Protocol> protocol) : fd_(fd), poller_(fd), protocol_(std::move(protocol)) {}

    [[nodiscard]] virtual std::unique_ptr<Connection> create_connection(SOCKET fd_) const = 0;

private:
    SOCKET                    fd_;
    Poller                    poller_;
    std::unique_ptr<Protocol> protocol_;
};

#endif //NEBLINA_SERVER_HH
