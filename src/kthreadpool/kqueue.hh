#ifndef NEBLINA_K_QUEUE_HH
#define NEBLINA_K_QUEUE_HH

#include <queue>
#include <map>

#include "key.hh"
#include "task.hh"

class KQueue {
public:
    void             sync_enqueue(Key key, Task task);
    std::queue<Task> sync_dequeue(Key key);

private:
    std::map<Key, std::queue<Task>> kqueue_;
};

#endif //NEBLINA_K_QUEUE_HH
