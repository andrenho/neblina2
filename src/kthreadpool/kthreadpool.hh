#ifndef NEBLINA_KEY_THREAD_POOL_HH
#define NEBLINA_KEY_THREAD_POOL_HH

#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

#include "key.hh"
#include "task.hh"
#include "syncqueue.hh"

class KThreadPool {
public:
    explicit KThreadPool(size_t thread_count);
    ~KThreadPool();

    void add_task(Key key, Task task);

    // non-copyable
    KThreadPool(KThreadPool const&) = delete;
    KThreadPool& operator=(KThreadPool const&) = delete;

private:
    size_t thread_count_;

    struct Thread {
        size_t          n;
        std::thread     thread;
        SyncQueue<Task> tasks {};
    };
    std::vector<std::unique_ptr<Thread>> threads_;
    std::atomic<bool>   running_ = true;
};

#endif //NEBLINA_KEY_THREAD_POOL_HH
