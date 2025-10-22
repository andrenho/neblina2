#include "socket.hh"

#include <cstring>
#include <string>

using namespace std::string_literals;

#include "util/exceptions/non_recoverable_exception.hh"

void Socket::mark_as_non_blocking()
{
#ifdef _WIN32
    unsigned long mode = 1;
    if (ioctlsocket(fd, FIONBIO, &mode) != 0)
        throw NonRecoverableException("error marking socket as non-blocking");
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw NonRecoverableException("fcntl: {}"s + strerror(errno));
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        throw NonRecoverableException("error marking socket as non-blocking");
#endif
}
