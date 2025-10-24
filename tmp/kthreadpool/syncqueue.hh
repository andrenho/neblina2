#ifndef NEBLINA_K_QUEUE_HH
#define NEBLINA_K_QUEUE_HH

#include <condition_variable>
#include <deque>
#include <thread>
#include <unordered_map>
#include <vector>

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class SyncQueue
{
public:
    SyncQueue() : queue_() , mutex_() , cond_() {}

    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(t);
        cond_.notify_one();
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(queue_.empty())
        {
            cond_.wait(lock);
            if (shutdown_)
                return {};
        }
        T val = queue_.front();
        queue_.pop();
        return val;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void finalize()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        shutdown_ = true;
        cond_.notify_one();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    bool shutdown_ = false;
};

#endif //NEBLINA_K_QUEUE_HH
