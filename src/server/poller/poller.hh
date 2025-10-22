#ifndef NEBLINA_POLLER_HH
#define NEBLINA_POLLER_HH

#include <chrono>
using namespace std::chrono_literals;

#include "util/socket.hh"

class Poller {
public:
    enum class EventType { NewClient, ClientDataReady, ClientDisconnected };

    struct Event {
        EventType type;
        SOCKET    fd;
    };

    explicit Poller(Socket const& server_socket);
    ~Poller();

    std::vector<Event> wait(std::chrono::milliseconds timeout=50ms);

    void add_client(Socket const* client_socket);
    void remove_client(Socket const* client_socket);

private:
    Socket const& server_socket_;

    struct Custom;
    std::unique_ptr<Custom> p;

    static constexpr size_t EVENT_BUFFER_SZ = 64;
};

#endif //NEBLINA_POLLER_HH
