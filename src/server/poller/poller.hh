#ifndef NEBLINA_POLLER_HH
#define NEBLINA_POLLER_HH

#include <chrono>
using namespace std::chrono_literals;

#include "util/socket.hh"

class Poller {
public:

    enum class EventType { NewConnection, DataReady, Disconnected };

    struct Event {
        EventType type;
        SOCKET    fd;
    };

    explicit Poller(Socket const& socket);

    std::vector<Event> wait(std::chrono::milliseconds timeout=50ms);

private:
    Socket const& socket_;
};

#endif //NEBLINA_POLLER_HH
