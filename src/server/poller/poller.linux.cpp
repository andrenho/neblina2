#include "poller.hh"

#include <cstring>
#include <cerrno>
#include <sys/epoll.h>

#include <string>
using namespace std::string_literals;

#include "util/exceptions/non_recoverable_exception.hh"

struct Poller::Custom {
    int epoll_fd;
};

Poller::Poller(Socket const& server_socket)
    : server_socket_(server_socket), p(new Poller::Custom{})
{
    p->epoll_fd = epoll_create1(0);
    if (p->epoll_fd < 0)
        throw NonRecoverableException("Could not initialize epoll: "s + strerror(errno));

    epoll_event event {};
    event.events = EPOLLIN;
    event.data.fd = server_socket_.fd;
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, server_socket_.fd, &event) < 0)
        throw NonRecoverableException("Could not initialize socket fd in epoll: "s + strerror(errno));
}

Poller::~Poller()
{
    close_socket(p->epoll_fd);
}

std::vector<Poller::Event> Poller::wait(std::chrono::milliseconds timeout)
{
    epoll_event events[EVENT_BUFFER_SZ];
    int n_events = epoll_wait(p->epoll_fd, events, EVENT_BUFFER_SZ, (int) timeout.count());

    if (n_events < 0) {
        if (errno == EINTR)
            return {};
        else
            throw NonRecoverableException("epoll error: "s + strerror(errno));
    }

    std::vector<Poller::Event> evs;

    for (int i = 0; i < n_events; ++i) {
        if (events[i].data.fd == server_socket_.fd) {
            evs.emplace_back(EventType::NewClient, server_socket_.fd);
        } else {
            if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                evs.emplace_back(EventType::ClientDisconnected, (SOCKET) events[i].data.fd);
            } else if (events[i].events & (EPOLLIN | EPOLLET)) {
                evs.emplace_back(EventType::ClientDataReady, (SOCKET) events[i].data.fd);
            }
        }
    }

    return evs;
}

void Poller::add_client(const Socket *client_socket)
{
    epoll_event event {};
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    event.data.fd = client_socket->fd;
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, client_socket->fd, &event) < 0)
        throw NonRecoverableException("Could not add client socket in epoll: "s + strerror(errno));
}

void Poller::remove_client(const Socket *client_socket)
{
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_DEL, client_socket->fd, nullptr) < 0)
        throw NonRecoverableException("Could not remove client socket from epoll: "s + strerror(errno));
}
