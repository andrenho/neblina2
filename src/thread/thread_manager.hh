#ifndef NEBLINA_THREAD_MANAGER_HH
#define NEBLINA_THREAD_MANAGER_HH

#include <memory>
#include <variant>
#include <vector>

#include "thread.hh"

enum class Threads { Single };
using ThreadCount = std::variant<Threads, int>;

class ThreadManager {
public:
    explicit ThreadManager(ThreadCount thread_count);

private:
    std::vector<std::unique_ptr<Thread>> threads_;
};

#endif //NEBLINA_THREAD_MANAGER_HH
