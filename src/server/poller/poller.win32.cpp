#include "poller.hh"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
using namespace std::string_literals;

struct Poller::Custom {
    std::vector<WSAPOLLFD> fds;
};

Poller::Poller(SOCKET server_fd)
    : server_fd_(server_fd), p(std::make_unique<Custom>())
{
    WSAPOLLFD poll_fd{};
    poll_fd.fd = server_fd_;
    poll_fd.events = POLLRDNORM; // readable → new client
    p->fds.push_back(poll_fd);
}

Poller::~Poller() = default;

std::vector<Poller::Event> Poller::wait(std::chrono::milliseconds timeout)
{
    int t = timeout.count() < 0 ? -1 : static_cast<int>(timeout.count());

    if (p->fds.empty())
        return {};

    int r = WSAPoll(p->fds.data(), static_cast<ULONG>(p->fds.size()), t);
    if (r < 0)
        throw std::runtime_error("WSAPoll failed"s);

    std::vector<Event> events;
    if (r == 0)
        return events; // timeout

    // index 0 = server socket (new connections)
    if (p->fds[0].revents & POLLRDNORM) {
        events.push_back({ EventType::NewClient, server_fd_ });
    }

    // client sockets
    for (size_t i = 1; i < p->fds.size(); ++i) {
        auto& fd = p->fds[i];

        if (fd.revents == 0)
            continue;

        // client readable
        if (fd.revents & POLLRDNORM) {
            events.push_back({ EventType::ClientDataReady, fd.fd });
        }

        // connection closed or error (Windows: POLLHUP usually implied)
        if (fd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
            events.push_back({ EventType::ClientDisconnected, fd.fd });
        }
    }

    return events;
}

void Poller::add_client(SOCKET client_fd)
{
    WSAPOLLFD poll_fd{};
    poll_fd.fd = client_fd;
    poll_fd.events = POLLRDNORM; // readable
    p->fds.push_back(poll_fd);
}
