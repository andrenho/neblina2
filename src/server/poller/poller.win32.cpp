#include "poller.hh"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdexcept>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std::string_literals;

#include "util/exceptions/non_recoverable_exception.hh"

struct Poller::Custom {
    HANDLE iocp;
};

Poller::Poller(Socket const& server_socket)
        : server_socket_(server_socket), p(new Poller::Custom{})
{
    p->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!p->iocp)
        throw NonRecoverableException("Could not initialize IOCP: "s + std::to_string(GetLastError()));

    HANDLE result = CreateIoCompletionPort(
            (HANDLE)server_socket_.fd,
            p->iocp,
            reinterpret_cast<ULONG_PTR>(server_socket_.fd),
            0
    );

    if (!result)
        throw NonRecoverableException("Could not associate server socket with IOCP: "s + std::to_string(GetLastError()));
}

Poller::~Poller()
{
    CloseHandle(p->iocp);
}

std::vector<Poller::Event> Poller::wait(std::chrono::milliseconds timeout)
{
    DWORD bytes_transferred;
    ULONG_PTR completion_key;
    LPOVERLAPPED overlapped = nullptr;

    DWORD wait_ms = timeout.count();
    BOOL ok = GetQueuedCompletionStatus(
            p->iocp,
            &bytes_transferred,
            &completion_key,
            &overlapped,
            wait_ms
    );

    std::vector<Poller::Event> evs;

    if (!ok) {
        DWORD err = GetLastError();
        if (err == WAIT_TIMEOUT)
            return {};
        if (err == ERROR_OPERATION_ABORTED)
            return {};
        throw NonRecoverableException("IOCP wait error: "s + std::to_string(err));
    }

    SOCKET fd = reinterpret_cast<SOCKET>(completion_key);

    if (fd == server_socket_.fd) {
        evs.emplace_back(EventType::NewClient, fd);
    } else {
        if (bytes_transferred == 0) {
            evs.emplace_back(EventType::ClientDisconnected, fd);
        } else {
            evs.emplace_back(EventType::ClientDataReady, fd);
        }
    }

    return evs;
}

void Poller::add_client(const Socket *client_socket)
{
    HANDLE result = CreateIoCompletionPort(
            (HANDLE)client_socket->fd,
            p->iocp,
            reinterpret_cast<ULONG_PTR>(client_socket->fd),
            0
    );

    if (!result)
        throw NonRecoverableException("Could not add client to IOCP: "s + std::to_string(GetLastError()));
}

void Poller::remove_client(const Socket *client_socket)
{
    (void) client_socket;
}