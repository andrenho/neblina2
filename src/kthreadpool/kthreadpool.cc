#include "kthreadpool.hh"

KThreadPool::KThreadPool(size_t thread_count)
{
}

KThreadPool::~KThreadPool()
{
}

void KThreadPool::add_task(uintptr_t key, Task task)
{
}

size_t KThreadPool::tasks_pending() const
{
    return 0;
}
