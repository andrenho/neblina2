#include "init.hh"

#include <string>

#include "src/util/socket.hh"
#include "util/exceptions/non_recoverable_exception.hh"

static Init init;

Init::Init()
{
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
        throw NonRecoverableException("WSAStartup failed: " + std::to_string(result));
#endif
}

Init::~Init()
{
#ifdef _WIN32
    WSACleanup();
#endif
}
