#include "doctest.h"

#include <atomic>

#include "kthreadpool/kthreadpool.hh"
#include "tests/echo/echo.hh"
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


TEST_SUITE("Load test")
{
    TEST_CASE("KThreadPool")
    {
        fprintf(stderr, "Load test: KThreadPool\n");

        std::atomic<size_t> i = 0;
        {
            KThreadPool ktpool(16);
            for (size_t j = 0; j < 16; ++j)
                for (size_t k = 0; k < 1000; ++k)
                    ktpool.add_task(j, [&i]() { ++i; return true; });
        }
        CHECK(i == 16000);
    }

    TEST_CASE("TCP Server")
    {
        fprintf(stderr, "Load test: TCP Server\n");

        logging_dest = stderr;

        std::thread t = run_server();
        while (!server_ready) {}

#define CLIENTS 1000

        std::vector<std::unique_ptr<TCPClient>> clients;
        for (size_t j = 0; j < CLIENTS; ++j)
            clients.push_back(std::make_unique<TCPClient>("127.0.0.1", PORT));
        for (size_t j = 0; j < CLIENTS; ++j)
            clients[j]->send("hello\r\n");
        for (size_t j = 0; j < CLIENTS; ++j) {
            std::string response = clients[j]->recv_spinlock(7, 100ms);
            CHECK(response == "hello\r\n");
        }

        server_running = false;
        t.join();
    }

    /*
    TEST_CASE("TCP Server")
    {
        fprintf(stderr, "Load test: TCP Server\n");

        logging_dest = stderr;

        std::thread t = run_server();
        while (!server_ready) {}

#define THREADS 2
#define CLIENTS 100

        std::thread tc[THREADS];
        for (size_t i = 0; i < THREADS; ++i) {
            tc[i] = std::thread([]() {
                std::vector<std::unique_ptr<TCPClient>> clients;
                for (size_t j = 0; j < CLIENTS; ++j)
                    clients.push_back(std::make_unique<TCPClient>("127.0.0.1", PORT));
                for (size_t j = 0; j < CLIENTS; ++j)
                    clients[j]->send("hello\r\n");
                for (size_t j = 0; j < CLIENTS; ++j) {
                    std::string response = clients[j]->recv_spinlock(7, 100ms);
                    CHECK(response == "hello\r\n");
                }
            });
        }
        for (size_t i = 0; i < 16; ++i)
            tc[i].join();

        server_running = false;
        t.join();
    }
     */
}