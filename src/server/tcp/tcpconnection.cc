#include "tcpconnection.hh"

#include <string>
#include <cstring>

using namespace std::string_literals;

std::string TCPConnection::recv() const
{
    static constexpr size_t RECV_BUF_SZ = 16 * 1024;
    std::string buf(RECV_BUF_SZ, 0);
    ssize_t r = ::recv(fd_, buf.data(), RECV_BUF_SZ, 0);
#ifdef _WIN32
    if (r == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            return "";
        }
        else {
            throw std::runtime_error("recv error: " + std::to_string(err));
        }
    }
#endif
    if (r < 0) {
        if (errno == EAGAIN)
            return "";
        else
            throw std::runtime_error("recv error: "s + strerror(errno));
    } else if (r == 0) {
        return "";
    } else {
        buf.resize(r);
        return buf;
    }
}

void TCPConnection::send(std::string const &data) const
{
    size_t pos = 0;
    while (pos < data.size()) {
        ssize_t r = ::send(fd_, data.data(), data.size(), 0);
        if (r < 0)
            throw std::runtime_error("send error: "s + strerror(errno));
        pos += r;
    }
}
