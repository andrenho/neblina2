#ifndef NEBLINA_KEY_THREAD_POOL_HH
#define NEBLINA_KEY_THREAD_POOL_HH

#include <cstddef>
#include <thread>
#include <vector>

#include "key.hh"
#include "task.hh"
#include "kqueue.hh"

class KThreadPool {
public:
    explicit KThreadPool(size_t thread_count);
    ~KThreadPool();

    void add_task(Key key, Task task);

    size_t queue_size() const { return kqueue_.queue_size(); }

    // non-copyable
    KThreadPool(KThreadPool const&) = delete;
    KThreadPool& operator=(KThreadPool const&) = delete;

private:
    KQueue                   kqueue_;
    std::vector<std::thread> threads_;
};

#endif //NEBLINA_KEY_THREAD_POOL_HH
