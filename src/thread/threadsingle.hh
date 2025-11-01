#ifndef NEBLINA_THREADSINGLE_HH
#define NEBLINA_THREADSINGLE_HH

#include <memory>
#include <unordered_map>

#include "thread.hh"
#include "util/socket.hh"

class ThreadSingle : public Thread {
public:
    void add_session(std::unique_ptr<Session> session) override;
    void remove_session(SOCKET fd) override;
    void data_available(SOCKET fd) override;
};


#endif //NEBLINA_THREADSINGLE_HH
