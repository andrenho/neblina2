#ifndef NEBLINA_SOCKET_HH
#define NEBLINA_SOCKET_HH

#ifdef _WIN32

#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "Ws2_32.lib")
#  define SOCKETOPT_YES char yes = '1';

typedef long ssize_t;

void socket_init();
void socket_finalize();

#define close_socket closesocket

#else

#  include <unistd.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <netinet/in.h>

#  define INVALID_SOCKET (-1)
#  define SOCKET_ERROR (-1)
#  define SOCKETOPT_YES int yes = 1;

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define socket_init() {}
#define socket_finalize() {}

#define close_socket close

#endif

#include <fcntl.h>

class Socket {
public:
    explicit Socket(SOCKET fd) : fd_(fd) {}
    ~Socket() { close_socket(fd_); }

    Socket(Socket&&) noexcept = default;
    Socket& operator=(Socket&&) noexcept = default;

    [[nodiscard]] SOCKET fd() const { return fd_; }

    // forbid copies
    Socket(const Socket&) = delete;
    Socket& operator=(Socket const&) = delete;

private:
    SOCKET fd_;
};

#endif //NEBLINA_SOCKET_HH
