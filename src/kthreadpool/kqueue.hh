#ifndef NEBLINA_K_QUEUE_HH
#define NEBLINA_K_QUEUE_HH

#include <condition_variable>
#include <deque>
#include <thread>
#include <unordered_map>
#include <vector>

#include "key.hh"
#include "task.hh"

class KQueue {
public:
    void             sync_enqueue(Key key, Task task);
    std::deque<Task> sync_dequeue();
    void             shutdown();

    size_t           queue_size() const;

    bool is_shutdown() const;

private:
    std::vector<Key>                          queue_;
    std::unordered_map<Key, std::deque<Task>> task_map_;
    std::condition_variable                   cond_;
    mutable std::mutex                        mutex_;
    bool                                      shutdown_ = false;
};

#endif //NEBLINA_K_QUEUE_HH
