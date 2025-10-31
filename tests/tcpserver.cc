#include "doctest.h"

#include <atomic>
#include <memory>
#include <thread>

#include "echo/echo.hh"
#include "server/tcpserver.hh"
#include "client/tcpclient.hh"

#define PORT 23456

TEST_SUITE("TCP Server")
{
    TEST_CASE("Single-threaded server and client")
    {
        logging_verbose = true;
        logging_dest = stderr;

        TCPServer server(PORT, false, std::make_unique<EchoProtocol>(), Thread::Single);
        TCPClient client("127.0.0.1", PORT);

        client.send("hello\r\n");
        std::this_thread::sleep_for(6ms);
        server.iterate();
        server.iterate();

        std::string response = client.recv_spinlock(7, 100ms);
        CHECK(response == "hello\r\n");
    }
}

#if 0

static std::atomic<bool> server_running;
static std::atomic<bool> server_ready;

static std::thread run_server()
{
    server_running = true;
    server_ready = false;
    return std::thread([]() {
        DBG("Creating server");
        auto server = TCPServer(PORT, false, std::make_unique<EchoProtocol>(), 2);
        server_ready = true;
        while (server_running)
            server.iterate();
        DBG("Destroying server");
    });
}

TEST_SUITE("TCP Server")
{
    TEST_CASE("Simple echo - singlethreaded")
    {
        logging_verbose = true;
        logging_dest = stderr;

        auto server = TCPServer(PORT, false, std::make_unique<EchoProtocol>(), 2);
        {
            TCPClient client1("127.0.0.1", PORT);
            TCPClient client2("127.0.0.1", PORT);

            client1.send("hello\r\n");
            client2.send("hellw\r\n");

            std::this_thread::sleep_for(10ms);

            for (size_t i = 0; i < 10; ++i)
                server.iterate();

            std::string response = client1.recv_spinlock(7, 100ms);
            CHECK(response == "hello\r\n");

            response = client2.recv_spinlock(7, 100ms);
            CHECK(response == "hellw\r\n");
        }
    }
    /*
    TEST_CASE("Simple echo")
    {
        logging_verbose = true;
        logging_dest = stderr;

        std::thread t = run_server();
        while (!server_ready) {}

        {
            TCPClient client1("127.0.0.1", PORT);
            TCPClient client2("127.0.0.1", PORT);

            client1.send("hello\r\n");
            client2.send("hellw\r\n");

            std::string response = client1.recv_spinlock(7, 100ms);
            CHECK(response == "hello\r\n");

            response = client2.recv_spinlock(7, 100ms);
            CHECK(response == "hellw\r\n");
        }

        server_running = false;
        t.join();
    }
     */
}

#endif