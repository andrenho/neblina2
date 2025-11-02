#include "threadmulti.hh"

#include <chrono>
using namespace std::chrono_literals;

ThreadMulti::ThreadMulti()
{
    thread_ = std::jthread([this](std::stop_token const& st) {
        while (!st.stop_requested()) {

            // wait for queue
            std::unique_lock lock(mutex_);
            cond_.wait_for(lock, 10ms);

            // bail out if queue empty (when deactivating the thread)
            if (queue_.empty())
                continue;

            // remove from queue
            SOCKET fd = queue_.front();
            queue_.pop_front();

            // execute interation
            auto it = sessions_.find(fd);
            if (it != sessions_.end())
                it->second->iteration();
        }
        finished_ = true;
    });
}

ThreadMulti::~ThreadMulti()
{
    thread_.request_stop();
    std::this_thread::sleep_for(15ms);

    std::lock_guard lock(mutex_);
    cond_.notify_one();
}

void ThreadMulti::add_session(std::unique_ptr<Session> session)
{
    std::lock_guard lock(mutex_);
    sessions_[session->fd()] = std::move(session);
}

void ThreadMulti::remove_session(SOCKET fd)
{
    std::lock_guard lock(mutex_);
    sessions_.erase(fd);
}

void ThreadMulti::data_available(SOCKET fd)
{
    std::lock_guard lock(mutex_);
    queue_.push_back(fd);
    cond_.notify_one();
}