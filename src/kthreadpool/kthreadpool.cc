#include "kthreadpool.hh"

KThreadPool::KThreadPool(size_t thread_count)
{
}

KThreadPool::~KThreadPool()
{
}

void KThreadPool::add_task(Key key, Task task)
{
    kqueue_.sync_enqueue(key, std::move(task));
}

size_t KThreadPool::tasks_pending() const
{
    return 0;
}
