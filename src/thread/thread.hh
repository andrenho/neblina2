#ifndef NEBLINA_THREAD_HH
#define NEBLINA_THREAD_HH

#include <memory>
#include "protocol/session.hh"

class Thread {
public:
    virtual ~Thread() = default;

    virtual void add_session(std::unique_ptr<Session> session) = 0;
    virtual void remove_session(SOCKET fd) = 0;

    virtual void data_available(SOCKET fd) = 0;

protected:
    Thread() = default;
};

#endif //NEBLINA_THREAD_HH
