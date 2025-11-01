#include "tcpserver.hh"

#include <string>
#include <cstring>

using namespace std::string_literals;

#include "util/exceptions/non_recoverable_exception.hh"
#include "tcpconnection.hh"

SOCKET TCPServer::create_listener(uint16_t port, bool open_to_world) const
{
    SOCKET listener = INVALID_SOCKET;

    // find internet address to bind
    struct addrinfo hints {};
    hints.ai_family = AF_INET;          // IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // use my IP

    const char* listen_to = open_to_world ? nullptr: "localhost";

    int rv;
    struct addrinfo* servinfo;
    char sport[15]; snprintf(sport, sizeof sport, "%d", port);
    if ((rv = getaddrinfo(listen_to, sport, &hints, &servinfo)) != 0)
        throw NonRecoverableException("getaddrinfo error: "s + gai_strerror(rv));

    // loop through all the results and bind to the first we can
    struct addrinfo* p;
    for(p = servinfo; p != nullptr; p = p->ai_next) {

        // open socket
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener == INVALID_SOCKET)
            throw NonRecoverableException("socket error: "s + strerror(errno));

        // set socket as reusable
        SOCKETOPT_YES
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR)
            throw NonRecoverableException("setsocket error: "s + strerror(errno));

        // bind to port
        if (bind(listener, p->ai_addr, (int) p->ai_addrlen) == SOCKET_ERROR) {
            close_socket(listener);
            continue;  // not possible, try next
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == nullptr)
        throw NonRecoverableException("failed to bind: "s + strerror(errno));

    if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
        throw NonRecoverableException("listen error: "s + strerror(errno));

    LOG("listening in port {}", port);
    DBG("with fd {}", listener);

    return listener;
}

SOCKET TCPServer::accept() const
{
    // accept connection
    struct sockaddr_storage remoteaddr {};
    socklen_t addrlen = sizeof remoteaddr;

    SOCKET client_fd = ::accept(fd_, (struct sockaddr *) &remoteaddr, &addrlen);
    if (client_fd == INVALID_SOCKET)
        throw NonRecoverableException("listen error: "s + strerror(errno));

    // find connecter IP/port
    char hoststr[1024] = "Unknown";
    char portstr[24] = "0";
    if (getnameinfo((struct sockaddr const*)(&remoteaddr), addrlen, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
        DBG("New connection from {}:{} as fd {}", hoststr, portstr, client_fd);

    // mark socket as non-blocking
    socket_mark_as_nonblocking(client_fd);
    return client_fd;
}

std::unique_ptr<Connection> TCPServer::create_connection(SOCKET fd_) const
{
    return std::make_unique<TCPConnection>(fd_);
}

