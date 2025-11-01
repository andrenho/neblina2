#ifndef NEBLINA_SERVER_HH
#define NEBLINA_SERVER_HH

#include "server/poller/poller.hh"
#include "protocol/protocol.hh"
#include "thread/thread_manager.hh"
#include "connection.hh"

#include <atomic>
#include <memory>

class Server {
public:
    virtual ~Server() { close_socket(fd_); }

    void iterate();
    void run()  { while (running_.load()) iterate(); }
    void stop() { running_.store(false); }

    [[nodiscard]] bool running() { return running_.load(); }

protected:
    explicit Server(SOCKET fd, std::unique_ptr<Protocol> protocol, ThreadCount thread_count)
        : fd_(fd), poller_(fd), protocol_(std::move(protocol)), thread_manager_(thread_count) {}

    [[nodiscard]] virtual std::unique_ptr<Connection> create_connection(SOCKET fd_) const = 0;
    [[nodiscard]] virtual SOCKET                      accept() const = 0;

    SOCKET                    fd_;

private:
    Poller                    poller_;
    std::unique_ptr<Protocol> protocol_;
    ThreadManager             thread_manager_;
    std::atomic<bool>         running_ = true;
};

#endif //NEBLINA_SERVER_HH
