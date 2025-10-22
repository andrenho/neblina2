#include "doctest.h"

#include <atomic>

#include "echo/echo.hh"
#include "server/tcpserver.hh"
#include "client/tcpclient.hh"

static std::atomic<bool> server_running;
static std::atomic<bool> server_ready;

#define PORT 23456

static std::thread run_server()
{
    server_running = true;
    server_ready = false;
    return std::thread([]() {
        auto server = TCPServer(PORT, false, std::make_unique<EchoProtocol>(), 8);
        server_ready = true;
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
        while (!server_ready) {}

        TCPClient client("127.0.0.1", PORT);
        client.send("hello\r\n");
        std::string response = client.recv_spinlock(7, 100ms);
        CHECK(response == "hello\r\n");

        server_running = false;
        t.join();
    }
}