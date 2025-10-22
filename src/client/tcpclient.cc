#include "tcpclient.hh"

#include <cstring>
#include <cerrno>
#include <format>

#include "util/log.hh"

using namespace std::string_literals;

TCPClient::TCPClient(std::string const& host, uint16_t port)
    : Client(open_connection(host, port))
{
}

static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

std::unique_ptr<Socket> TCPClient::open_connection(std::string const& host, uint16_t port) const
{
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    struct addrinfo* servinfo;
    std::string sport = std::to_string(port);
    if ((rv = getaddrinfo(host.c_str(), sport.c_str(), &hints, &servinfo)) != 0)
        throw std::runtime_error(std::format("getaddrinfo: {} // {}", strerror(errno), gai_strerror(rv)));

    SOCKET sockfd = INVALID_SOCKET;
    struct addrinfo* p;
    char address[INET6_ADDRSTRLEN];
    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, sizeof address);
        DBG("client: attempting connection to {}:{}", address, sport);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            DBG("connect() to {}:{} failed: {}", address, sport, strerror(errno));
            close_socket(sockfd);
            continue;
        }

        break;
    }

    if (p == nullptr)
        throw std::runtime_error("Failed to connect.");

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, sizeof address);
    DBG("client: connected to {}:{}", address, sport);

    freeaddrinfo(servinfo);

    auto socket = std::make_unique<Socket>(sockfd);
    socket->mark_as_non_blocking();
    return socket;
}

void TCPClient::send(std::string const& data) const
{
    size_t pos = 0;
    while (pos < data.size()) {
        ssize_t r = ::send(socket_->fd, data.data(), data.size(), 0);
        if (r < 0)
            throw std::runtime_error("send error: "s + strerror(errno));
        pos += r;
    }
}

std::optional<std::string> TCPClient::recv(size_t n_bytes) const
{
    std::string buf(n_bytes, 0);
    ssize_t r = ::recv(socket_->fd, buf.data(), n_bytes, 0);
    if (r < 0) {
        if (errno == EAGAIN)
            return {};
        else
            throw std::runtime_error("recv error: "s + strerror(errno));
    } else if (r == 0) {
        return "";
    } else {
        buf.resize(r);
        return buf;
    }
}
