#ifndef SERVERTHREAD_HH
#define SERVERTHREAD_HH

#include "protocol/session.hh"
#include "util/pcthread.hh"
#include "util/socket.hh"

class Server;

class ServerThread : public ProducerConsumerThread<SOCKET> {
public:
    explicit ServerThread(Server const* server, size_t n)
        : ProducerConsumerThread(std::string("Server thread #") + std::to_string(n)), server_(server) {}

    void add_session(std::unique_ptr<Session> session);
    void remove_socket(SOCKET fd);

protected:
    void action(SOCKET&& fd) override;

private:
    Server const* server_;
};

#endif //SERVERTHREAD_HH
