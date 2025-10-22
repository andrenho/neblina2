#include "client.hh"

#include <stdexcept>

using sc = std::chrono::steady_clock;

std::string Client::recv_spinlock(std::chrono::milliseconds timeout) const
{
    auto start = sc::now();
    while (sc::now() < (start + timeout)) {
        auto r = recv();
        if (r)
            return *r;
    }

    throw std::runtime_error("Recv timeout");
}

std::future<std::string> Client::recv_future(std::chrono::milliseconds ms) const
{
    return std::async(std::launch::async, [this, ms]() {
        return recv_spinlock(ms);
    });
}
