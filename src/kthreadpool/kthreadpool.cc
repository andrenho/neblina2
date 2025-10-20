#include "kthreadpool.hh"

KThreadPool::KThreadPool(size_t thread_count)
    : thread_count_(thread_count)
{
    for (size_t i = 0; i < thread_count; ++i) {
        auto t = std::make_unique<Thread>();
        t->n = i;
        t->thread = std::thread([this, &tasks = t->tasks, &n = t->n]() {
            while (running_) {
                auto task = tasks.dequeue();
                if (!task)
                    break;  // shutdown signal
                task();
            }
        });
        threads_.emplace_back(std::move(t));
    }
}

void KThreadPool::add_task(Key key, Task task)
{
    size_t partition = key % thread_count_;
    threads_.at(partition)->tasks.enqueue(std::move(task));
}

KThreadPool::~KThreadPool()
{
    for (auto& t: threads_)
        while (t->tasks.size() > 0)
            ;
    running_ = false;
    for (auto& t: threads_) {
        t->tasks.finalize();
        t->thread.join();
    }
}
