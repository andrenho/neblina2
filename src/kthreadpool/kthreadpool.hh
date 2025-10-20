#ifndef NEBLINA_KEY_THREAD_POOL_HH
#define NEBLINA_KEY_THREAD_POOL_HH

#include <cstddef>

#include "key.hh"
#include "task.hh"
#include "kqueue.hh"

class KThreadPool {
public:
    explicit KThreadPool(size_t thread_count);
    ~KThreadPool();

    void add_task(Key key, Task task);

    [[nodiscard]] size_t tasks_pending() const;

    // non-copyable
    KThreadPool(KThreadPool const&) = delete;
    KThreadPool& operator=(KThreadPool const&) = delete;

private:
    KQueue kqueue_;
};

#endif //NEBLINA_KEY_THREAD_POOL_HH
