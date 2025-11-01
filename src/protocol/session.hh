#ifndef NEBLINA_SESSION_HH
#define NEBLINA_SESSION_HH

#include <memory>
#include <string>

#include "server/connection.hh"

class Session {
public:
    virtual ~Session() = default;

    virtual void iteration();

    [[nodiscard]] SOCKET fd() const { return connection_->fd(); }

protected:
    explicit Session(std::unique_ptr<Connection> connection) : connection_(std::move(connection)) {}

    virtual std::string process(std::string const& data) = 0;

    std::unique_ptr<Connection> connection_;
};

#endif //NEBLINA_SESSION_HH
