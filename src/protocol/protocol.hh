#ifndef NEBLINA_PROTOCOL_HH
#define NEBLINA_PROTOCOL_HH

#include <memory>

#include "session.hh"

class Protocol {
public:
    virtual ~Protocol() = default;

    [[nodiscard]] virtual std::unique_ptr<Session> create_session(std::unique_ptr<Connection> connection) const = 0;

protected:
    Protocol() = default;
};

#endif //NEBLINA_PROTOCOL_HH
