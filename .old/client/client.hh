#ifndef NEBLINA_CLIENT_HH
#define NEBLINA_CLIENT_HH

#include <chrono>
#include <expected>
#include <future>
#include <optional>
#include <memory>

#include "util/socket.hh"

using namespace std::chrono_literals;

class Client {
public:
    virtual ~Client() = default;

    virtual void send(std::string const& data) const = 0;

    [[nodiscard]] std::optional<std::string> recv_spinlock(size_t n_bytes, std::chrono::milliseconds timeout) const;
    [[nodiscard]] std::future<std::string> recv_future(std::chrono::milliseconds timeout=30s) const;

protected:
    explicit Client(std::unique_ptr<Socket> socket) : socket_(std::move(socket)) {}

    [[nodiscard]] virtual std::string recv(size_t n_bytes) const = 0;

    std::unique_ptr<Socket> socket_;
};


#endif //NEBLINA_CLIENT_HH
