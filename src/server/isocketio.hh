#ifndef NEBLINA_ISOCKETIO_HH
#define NEBLINA_ISOCKETIO_HH

#include <string>
#include "util/socket.hh"

class ISocketIO {
public:
    // these 2 methods are called by a thread - they need to be thread safe!
    virtual std::string recv(SOCKET fd) const = 0;
    virtual void        send(SOCKET fd, std::string const &data) const = 0;
};

#endif
