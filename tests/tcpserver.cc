#include "doctest.h"

#include <atomic>

#include "echo/echo.hh"
#include "server/tcpserver.hh"

static std::atomic<bool> server_running;

static std::thread run_server()
{
    server_running = true;
    return std::thread([]() {
        auto server = TCPServer(23456, false, std::make_unique<EchoProtocol>(), 8);
        while (server_running)
            server.iterate();
    });
}

TEST_SUITE("TCP Server")
{
    TEST_CASE("Simple echo")
    {
        logging_verbose = true;
        std::thread t = run_server();
        std::this_thread::sleep_for(5s);
        server_running = false;
        t.join();
    }
}