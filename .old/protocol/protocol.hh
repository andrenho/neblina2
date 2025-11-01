#ifndef NEBLINA_PROTOCOL_HH
#define NEBLINA_PROTOCOL_HH

#include <memory>

#include "session.hh"
#include "util/socket.hh"

class Protocol {
public:
    virtual ~Protocol() = default;

    [[nodiscard]] virtual std::unique_ptr<Session> create_session(std::unique_ptr<Socket> socket) const = 0;
};


#endif //NEBLINA_PROTOCOL_HH
