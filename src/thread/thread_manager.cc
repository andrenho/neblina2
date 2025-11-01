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

void ThreadManager::add_session(std::unique_ptr<Session> session)
{
    threads_.at(session->fd() % threads_.size())->add_session(std::move(session));
}

void ThreadManager::remove_session(SOCKET fd)
{
    threads_.at(fd % threads_.size())->remove_session(fd);
}

void ThreadManager::data_available(SOCKET fd)
{
    threads_.at(fd % threads_.size())->data_available(fd);
}
