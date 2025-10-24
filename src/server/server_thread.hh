#ifndef NEBLINA_SERVER_THREAD_HH
#define NEBLINA_SERVER_THREAD_HH

#include <memory>
#include <unordered_map>

#include "util/socket.hh"
#include "protocol/session.hh"

class ServerThread {
public:
    explicit ServerThread(class Server* server);

    ~ServerThread();

    void take_socket_ownership(std::unique_ptr<Socket> socket);
    void remove_socket(SOCKET fd);

    void new_data_ready(SOCKET fd);

private:
    class Server* server_;
    std::unordered_map<SOCKET, std::unique_ptr<Session>> sessions_;
};


#endif //NEBLINA_SERVER_THREAD_HH
