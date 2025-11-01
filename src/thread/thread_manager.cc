#include "thread_manager.hh"
#include "threadmulti.hh"
#include "threadsingle.hh"

ThreadManager::ThreadManager(ThreadCount thread_count)
{
    if (int const* n = std::get_if<int>(&thread_count)) {
        for (int i = 0; i < *n; ++i)
            threads_.push_back(std::make_unique<ThreadMulti>());
    } else {
        threads_.push_back(std::make_unique<ThreadSingle>());
    }
}
