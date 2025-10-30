#ifndef NEBLINA_SERVER_HH
#define NEBLINA_SERVER_HH

#include <atomic>
#include <memory>
#include <vector>

#include "protocol/protocol.hh"
#include "util/socket.hh"
#include "server/poller/poller.hh"
#include "serverthread.hh"
#include "isocketio.hh"

class Server : public ISocketIO {
public:
    virtual ~Server() { finalize(); }

    void iterate();
    void run();

    [[nodiscard]] bool running() { return running_.load(); }

protected:
    Server(std::unique_ptr<Protocol> protocol, std::unique_ptr<Socket>, size_t n_threads);

    [[nodiscard]] virtual std::unique_ptr<Socket> accept_new_connection() const = 0;

    void finalize();

    std::unique_ptr<Socket>   server_socket_;

private:
    void handle_new_client();
    [[nodiscard]] size_t thread_hash(SOCKET fd) const;

    Poller                                      poller_;
    std::unique_ptr<Protocol>                   protocol_;
    std::atomic<bool>                           running_ = true;
    std::vector<std::unique_ptr<ServerThread>>  server_threads_;
};


#endif //NEBLINA_SERVER_HH
