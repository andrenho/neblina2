#include "serverthread.hh"

ServerThread::~ServerThread()
{
    std::lock_guard lock(mutex_);
    sessions_.clear();
}

void ServerThread::action(SOCKET&& fd)
{
    if (fd == INVALID_SOCKET)
        return;

    auto session = find_session(fd);
    if (session) {
        std::string request = io_.recv(fd);
        std::string response = (*session)->new_data(request);
        if (!response.empty())
            io_.send((*session)->socket().fd, response);
    }
}

void ServerThread::add_session(std::unique_ptr<Session> session)
{
    std::lock_guard lock(mutex_);
    sessions_[session->socket().fd] = std::move(session);
}

void ServerThread::remove_socket(SOCKET fd)
{
    std::lock_guard lock(mutex_);
    sessions_.erase(fd);
}

std::optional<Session *> ServerThread::find_session(SOCKET fd)
{
    std::lock_guard lock(mutex_);
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return {};
    return it->second.get();
}
