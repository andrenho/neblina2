#ifndef NEBLINA_THREADMULTI_HH
#define NEBLINA_THREADMULTI_HH

#include "thread.hh"

class ThreadMulti : public Thread {
public:
    void add_session(std::unique_ptr<Session> session) override;
    void remove_session(SOCKET fd) override;
    void data_available(SOCKET fd) override;
};

#endif //NEBLINA_THREADMULTI_HH
