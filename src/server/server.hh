#ifndef NEBLINA_SERVER_HH
#define NEBLINA_SERVER_HH

#include "server/poller/poller.hh"
#include "protocol/protocol.hh"
#include "thread/thread_manager.hh"
#include "connection.hh"

#include <memory>

class Server {
public:
    virtual ~Server() { close_socket(fd_); }

protected:
    explicit Server(SOCKET fd, std::unique_ptr<Protocol> protocol, ThreadCount thread_count)
        : fd_(fd), poller_(fd), protocol_(std::move(protocol)), thread_manager_(thread_count) {}

    [[nodiscard]] virtual std::unique_ptr<Connection> create_connection(SOCKET fd_) const = 0;

private:
    SOCKET                    fd_;
    Poller                    poller_;
    std::unique_ptr<Protocol> protocol_;
    ThreadManager             thread_manager_;
};

#endif //NEBLINA_SERVER_HH
