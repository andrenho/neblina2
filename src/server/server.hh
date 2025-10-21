#ifndef NEBLINA_SERVER_HH
#define NEBLINA_SERVER_HH

#include <atomic>
#include <memory>

#include "kthreadpool/kthreadpool.hh"
#include "protocol/protocol.hh"
#include "util/socket.hh"
#include "server/poller/poller.hh"

class Server {
public:
    virtual ~Server() = default;

    void iterate();
    void run();

    void terminate() { running_ = false; }

    [[nodiscard]] bool running() { return running_.load(); }

protected:
    explicit Server(std::unique_ptr<Protocol> protocol, Socket socket, size_t n_threads);

    Socket                    server_socket_;
    KThreadPool               kthreadpool_;
    std::unique_ptr<Protocol> protocol_;
    Poller                    poller_;
    std::atomic<bool>         running_ = true;
};


#endif //NEBLINA_SERVER_HH
