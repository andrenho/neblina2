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

Poller::Poller(SOCKET socket_fd)
    : server_fd_(socket_fd), p(std::make_unique<Poller::Custom>())
{
    p->kqueue_fd = kqueue();
    if (p->kqueue_fd < 0)
        throw NonRecoverableException("Could not initialize kqueue: "s + strerror(errno));

    struct kevent event {};
    EV_SET(&event, server_fd_, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
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
        if (fd == server_fd_) {
            evs.emplace_back(EventType::NewClient, server_fd_);
        } else if (events[i].flags & EV_EOF) {
            evs.emplace_back(EventType::ClientDisconnected, fd);

            struct kevent del_event {};
            EV_SET(&del_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            if (kevent(p->kqueue_fd, &del_event, 1, nullptr, 0, nullptr))
                throw NonRecoverableException("Could not remove client socket from kqueue: "s + strerror(errno));
        } else {
            evs.emplace_back(EventType::ClientDataReady, fd);
        }
    }

    return evs;
}

void Poller::add_client(SOCKET client_fd)
{
    struct kevent event {};
    EV_SET(&event, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(p->kqueue_fd, &event, 1, nullptr, 0, nullptr) < 0)
        throw NonRecoverableException("Could not add client socket in kqueue: "s + strerror(errno));
}