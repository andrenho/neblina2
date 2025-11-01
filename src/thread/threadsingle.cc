#include "threadsingle.hh"

void ThreadSingle::add_session(std::unique_ptr<Session> session)
{
    sessions_[session->fd()] = std::move(session);
}

void ThreadSingle::remove_session(SOCKET fd)
{
    sessions_.erase(fd);
}

void ThreadSingle::data_available(SOCKET fd)
{
    auto it = sessions_.find(fd);
    if (it != sessions_.end())
        it->second->iteration();
}
