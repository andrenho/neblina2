#ifndef NEBLINA_SESSION_HH
#define NEBLINA_SESSION_HH

#include <memory>

#include "util/socket.hh"

class Session {
public:
    explicit Session(std::unique_ptr<Socket> socket) : socket_(std::move(socket)) {}
    virtual ~Session() = default;

    [[nodiscard]] Socket const& socket() const { return *socket_; }

private:
    std::unique_ptr<Socket> socket_;
};

#endif //NEBLINA_SESSION_HH
