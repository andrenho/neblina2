#include "client.hh"

#include <stdexcept>

using sc = std::chrono::steady_clock;

std::optional<std::string> Client::recv_spinlock(size_t n_bytes, std::chrono::milliseconds timeout) const
{
    std::string buffer {};

    auto start = sc::now();
    while (sc::now() < (start + timeout)) {
        auto r = recv(n_bytes - buffer.length());
        if (!r.empty()) {
            buffer += r;
            if (buffer.length() >= n_bytes)
                return buffer;
        }
    }

    return {};
}

std::future<std::string> Client::recv_future(std::chrono::milliseconds ms) const
{
    static constexpr size_t BUF_SZ = 16 * 1024;
    return std::async(std::launch::async, [this, ms]() {
        auto r = recv_spinlock(BUF_SZ, ms);
        return r.has_value() ? *r : "";
    });
}
