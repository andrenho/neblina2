#ifndef NEBLINA_THREADMULTI_HH
#define NEBLINA_THREADMULTI_HH

#include <condition_variable>
#include <deque>
#include <thread>

#include "thread.hh"

class ThreadMulti : public Thread {
public:
    ThreadMulti();
    ~ThreadMulti();

    void add_session(std::unique_ptr<Session> session) override;
    void remove_session(SOCKET fd) override;
    void data_available(SOCKET fd) override;

private:
    std::jthread            thread_;
    std::mutex              mutex_;
    std::deque<SOCKET>      queue_;
    std::condition_variable cond_;
    std::atomic<bool>       finished_ = false;
};

#endif //NEBLINA_THREADMULTI_HH
