#ifndef NEBLINA_SERVER_HH
#define NEBLINA_SERVER_HH

#include <atomic>
#include <memory>
#include <unordered_map>

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

    // these 4 methods are called by a thread - they need to be thread safe!
    void                handle_client_data_ready(SOCKET fd);
    void                handle_client_disconnected(SOCKET fd);
    virtual std::string recv(Session const &session) const = 0;
    virtual void        send(Session const &session, std::string const &data) const = 0;


    std::unique_ptr<Socket>   server_socket_;

private:
    void handle_new_client();

    Poller                    poller_;
    std::unique_ptr<Protocol> protocol_;
    std::atomic<bool>         running_ = true;
    std::unordered_map<SOCKET, std::unique_ptr<Session>> sessions_;
    KThreadPool               kthreadpool_;
};


#endif //NEBLINA_SERVER_HH
