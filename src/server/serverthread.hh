#ifndef SERVERTHREAD_HH
#define SERVERTHREAD_HH

#include "protocol/session.hh"
#include "util/pcthread.hh"
#include "util/socket.hh"
#include "isocketio.hh"

class ServerThread : public ProducerConsumerThread<SOCKET> {
public:
    explicit ServerThread(ISocketIO const* io, size_t n)
        : ProducerConsumerThread(std::string("Server thread #") + std::to_string(n)), io_(io) {}

    void add_session(std::unique_ptr<Session> session);
    void remove_socket(SOCKET fd);

protected:
    void action(SOCKET&& fd) override;

private:
    ISocketIO const* io_;
};

#endif //SERVERTHREAD_HH
