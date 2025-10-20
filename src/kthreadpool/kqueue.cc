#include "kqueue.hh"

#include <algorithm>
#include <mutex>

void KQueue::sync_enqueue(Key key, Task task)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (shutdown_)
        return;

    auto it = std::ranges::find(queue_, key);
    if (it == queue_.end())
        queue_.push_back(key);

    printf("Enqueue %zu: ", key); for (Key key: queue_) printf("%zu ", key); printf("\n");

    task_map_[key].emplace_back(std::move(task));

    cond_.notify_one();
}

std::deque<Task> KQueue::sync_dequeue()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() { return shutdown_ || !queue_.empty(); });

    // handle shutdown
    if (shutdown_ && queue_.empty()) {
        return {};
    }

    Key key_on_front = queue_.front();
    queue_.erase(queue_.begin());  // pop from front

    printf("Dequeue %zu: ", key_on_front); for (Key key: queue_) printf("%zu ", key); printf("\n");

    auto it = task_map_.find(key_on_front);
    if (it != task_map_.end()) {
        std::deque<Task> queue_task = std::move(it->second);
        task_map_.erase(it);
        return queue_task;
    } else {
        return {};
    }
}

size_t KQueue::queue_size() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
}


void KQueue::shutdown()
{
    std::unique_lock<std::mutex> lock(mutex_);
    shutdown_ = true;
    cond_.notify_all();  // Wake up all waiting threads
}

bool KQueue::is_shutdown() const
{
    std::scoped_lock lock(mutex_);
    return shutdown_;
}
