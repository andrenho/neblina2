#ifndef NEBLINA_CONNECTION_HH
#define NEBLINA_CONNECTION_HH

#include "util/socket.hh"

class Connection {
public:
    virtual ~Connection() { close_socket(fd_); }

    [[nodiscard]] virtual std::string recv() const = 0;
    virtual void                      send(std::string const &data) const = 0;

    [[nodiscard]] SOCKET fd() const { return fd_; }

protected:
    explicit Connection(SOCKET fd) : fd_(fd) {}

    const SOCKET fd_;
};

#endif //NEBLINA_CONNECTION_HH
