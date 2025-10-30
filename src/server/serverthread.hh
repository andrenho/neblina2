#ifndef SERVERTHREAD_HH
#define SERVERTHREAD_HH

#include <unordered_map>
#include <memory>

#include "protocol/session.hh"
#include "util/pcthread.hh"
#include "util/socket.hh"
#include "isocketio.hh"

class ServerThread : public ProducerConsumerThread<SOCKET> {
public:
    explicit ServerThread(ISocketIO const& io, size_t thread_n)
        : ProducerConsumerThread(std::string("Server thread #") + std::to_string(thread_n)), io_(io) {}

    void add_session(std::unique_ptr<Session> session);
    void remove_socket(SOCKET fd);

protected:
    void action(SOCKET&& fd) override;

private:
    ISocketIO const& io_;
    std::unordered_map<SOCKET, std::unique_ptr<Session>> sessions_;
    std::mutex mutex_;

    Session* find_session(SOCKET fd);
};

#endif //SERVERTHREAD_HH
