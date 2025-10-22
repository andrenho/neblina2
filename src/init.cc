#include "init.hh"

#include <cstring>
#include <cerrno>
#include <string>

#ifndef _WIN32
#include <sys/resource.h>
#endif

#include "src/util/socket.hh"
#include "util/exceptions/non_recoverable_exception.hh"

using namespace std::string_literals;

static Init init;

Init::Init()
{
    logging_color = "0";
    service_name = "(undefined)";
    logging_verbose = false;
    logging_dest = stdout;

    // initialize Winsock
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
        throw NonRecoverableException("WSAStartup failed: " + std::to_string(result));
#endif

    // increase max number of files
#ifndef _WIN32
    constexpr size_t desired_rl = 65535;

    rlimit rl {};
    getrlimit(RLIMIT_NOFILE, &rl);
    if (rl.rlim_cur < desired_rl) {
        rl.rlim_cur = desired_rl;
        if (setrlimit(RLIMIT_NOFILE, &rl) != 0)
            LOG("Could not set file limit: {}", strerror(errno));
    }
#endif
}

Init::~Init()
{
#ifdef _WIN32
    WSACleanup();
#endif
}
