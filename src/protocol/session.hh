#ifndef NEBLINA_SESSION_HH
#define NEBLINA_SESSION_HH

#include "server/connection.hh"

class Session {
public:
    virtual ~Session() = default;

protected:
    explicit Session(std::unique_ptr<Connection> connection) : connection_(std::move(connection)) {}

    std::unique_ptr<Connection> connection_;
};

#endif //NEBLINA_SESSION_HH
