#include "kthreadpool.hh"

KThreadPool::KThreadPool(size_t thread_count)
{
    for (size_t i = 0; i < thread_count; ++i) {
        threads_.emplace_back([this, i]() {
            while (true) {
                std::deque<Task> tasks = kqueue_.sync_dequeue();

                // Empty deque + shutdown = exit
                if (tasks.empty()) {
                    if (kqueue_.is_shutdown())
                        break;
                    else
                        continue;
                }

                for (Task& task: tasks) {
                    if (!task())
                        return;
                }
            }
        });
    }
}

void KThreadPool::add_task(Key key, Task task)
{
    kqueue_.sync_enqueue(key, std::move(task));
}

KThreadPool::~KThreadPool()
{
    kqueue_.shutdown();
    for (std::thread& t: threads_)
        t.join();
}
