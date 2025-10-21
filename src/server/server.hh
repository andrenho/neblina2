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
    Server(std::unique_ptr<Protocol> protocol, std::unique_ptr<Socket>, size_t n_threads);

    [[nodiscard]] virtual std::unique_ptr<Socket> accept_new_connection() const = 0;

    std::unique_ptr<Socket>   server_socket_;

private:
    void handle_new_client();
    void handle_client_data_ready(SOCKET fd);
    void handle_client_disconnected(SOCKET fd);

    KThreadPool               kthreadpool_;
    Poller                    poller_;
    std::unique_ptr<Protocol> protocol_;
    std::atomic<bool>         running_ = true;
};


#endif //NEBLINA_SERVER_HH
