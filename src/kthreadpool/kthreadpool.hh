#ifndef NEBLINA_KEY_THREAD_POOL_HH
#define NEBLINA_KEY_THREAD_POOL_HH

#include <cstddef>
#include <cstdint>

#include <functional>

using Task = std::function<bool()>;

class KThreadPool {
public:
    KThreadPool(size_t thread_count);
    ~KThreadPool();

    void add_task(uintptr_t key, Task task);

    size_t tasks_pending() const;

private:
    KThreadPool(KThreadPool const&) = delete;
    KThreadPool& operator=(KThreadPool const&) = delete;
};

#endif //NEBLINA_KEY_THREAD_POOL_HH
