#include "server_thread.hh"

#include <cassert>

#include "server.hh"
#include "util/log.hh"

ServerThread::ServerThread(class Server *server)
    : server_(server)
{
    DBG("creating server thread");
    thread_ = std::thread([this](){
        while (running_) {
            SOCKET fd = pop_from_queue();
            if (fd != INVALID_SOCKET)
                process_socket(fd);
        }
    });
}

ServerThread::~ServerThread()
{
    running_ = false;
    DBG("closing server thread");

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        cond_.notify_one();
    }

    thread_.join();
}

void ServerThread::add_session(std::unique_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_[session->socket().fd] = std::move(session);
}

void ServerThread::remove_socket(SOCKET fd)
{
    // remove all references in queue
    std::lock_guard<std::mutex> lock2(queue_mutex_);
    queue_unique_.erase(fd);
    auto it = std::find(queue_.begin(), queue_.end(), fd);
    if (it != queue_.end())
        queue_.erase(it);

    // erase session
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(fd);
}

void ServerThread::process_socket(SOCKET fd)
{
    if (fd == INVALID_SOCKET)
        return;

    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto& session = sessions_.at(fd);
    std::string request = server_->recv(session->socket().fd);
    std::string response = session->new_data(request);
    server_->send(session->socket().fd, response);
}

void ServerThread::new_data_ready(SOCKET fd)
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!queue_unique_.contains(fd)) {
        queue_unique_.insert(fd);
        queue_.push_back(fd);
        cond_.notify_one();
    }
}

SOCKET ServerThread::pop_from_queue()
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    cond_.wait(lock);
    if (!running_)
        return INVALID_SOCKET;
    assert(!queue_.empty());
    SOCKET fd = queue_.front();
    queue_.pop_front();
    queue_unique_.erase(fd);
    return fd;
}


