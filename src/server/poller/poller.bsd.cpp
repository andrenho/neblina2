#include "poller.hh"

#include <cstring>
#include <cerrno>
#include <sys/event.h>

#include <string>
#include <vector>
using namespace std::string_literals;

#include "util/exceptions/non_recoverable_exception.hh"

struct Poller::Custom {
    int kqueue_fd;
};

Poller::Poller(Socket const& server_socket)
    : server_socket_(server_socket), p(new Poller::Custom{})
{
    p->kqueue_fd = kqueue();
    if (p->kqueue_fd < 0)
        throw NonRecoverableException("Could not initialize kqueue: "s + strerror(errno));

    struct kevent event {};
    EV_SET(&event, server_socket.fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
    if (kevent(p->kqueue_fd, &event, 1, nullptr, 0, nullptr) < 0)
        throw NonRecoverableException("Could not initialize socket fd in kqueue: "s + strerror(errno));
}

Poller::~Poller()
{
    close_socket(p->kqueue_fd);
}

std::vector<Poller::Event> Poller::wait(std::chrono::milliseconds timeout)
{
    struct kevent events[EVENT_BUFFER_SZ];
    timespec timeout_c = { .tv_sec = 0, .tv_nsec = timeout.count() * 1000000 };

    int n_events = kevent(p->kqueue_fd, nullptr, 0, events, EVENT_BUFFER_SZ, &timeout_c);
    if (n_events < 0 && errno != EINTR)
        throw NonRecoverableException("epoll error: "s + strerror(errno));

    std::vector<Poller::Event> evs;

    for (int i = 0; i < n_events; ++i) {
        auto fd = (SOCKET) events[i].ident;
        if (fd == server_socket_.fd)
            evs.emplace_back(EventType::NewClient, server_socket_.fd);
        else if (events[i].flags & EV_EOF)
            evs.emplace_back(EventType::ClientDisconnected, fd);
        else
            evs.emplace_back(EventType::ClientDataReady, fd);
    }

    return evs;
}

void Poller::add_client(const Socket *client_socket)
{
    struct kevent event {};
    EV_SET(&event, client_socket->fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(p->kqueue_fd, &event, 1, nullptr, 0, nullptr) < 0)
        throw NonRecoverableException("Could not add client socket in kqueue: "s + strerror(errno));
}

void Poller::remove_client(const Socket *client_socket)
{
    struct kevent del_event {};
    EV_SET(&del_event, client_socket->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if (kevent(p->kqueue_fd, &del_event, 1, nullptr, 0, nullptr))
        throw NonRecoverableException("Could not remove client socket from kqueue: "s + strerror(errno));
}
