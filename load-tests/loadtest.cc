#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../tests/doctest.h"

#include <atomic>

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
        auto server = TCPServer(PORT, false, std::make_unique<EchoProtocol>(), 8u);
        server_ready = true;
        while (server_running)
            server.iterate();
        server.finalize();
    });
}


TEST_SUITE("Load test")
{
    TEST_CASE("TCP Server - single threaded clients")
    {
        fprintf(stderr, "Load test: TCP Server\n");

        logging_dest = stderr;

        std::thread t = run_server();
        while (!server_ready) {}
        std::this_thread::sleep_for(50ms);

#define CLIENTS 10000

        std::vector<std::unique_ptr<TCPClient>> clients;
        for (size_t j = 0; j < CLIENTS; ++j)
            clients.push_back(std::make_unique<TCPClient>("127.0.0.1", PORT));
        for (size_t j = 0; j < CLIENTS; ++j)
            clients[j]->send("hello\r\n");
        for (size_t j = 0; j < CLIENTS; ++j) {
            std::string response = clients[j]->recv_spinlock(7, 1000ms).value_or("");
            CHECK(response == "hello\r\n");
        }

        server_running = false;
        if (t.joinable())
            t.join();
    }

    TEST_CASE("TCP Server - multithreaded clients")
    {
        fprintf(stderr, "Load test: TCP Server\n");

        logging_dest = stderr;

        std::thread t = run_server();
        while (!server_ready) {}
        std::this_thread::sleep_for(50ms);

#define THREADS 8
#define N_CLIENTS 1000

        std::thread tc[THREADS];
        for (size_t i = 0; i < THREADS; ++i) {
            tc[i] = std::thread([]() {
                std::vector<std::unique_ptr<TCPClient>> clients;
                for (size_t j = 0; j < N_CLIENTS; ++j)
                    clients.push_back(std::make_unique<TCPClient>("127.0.0.1", PORT));
                for (size_t j = 0; j < N_CLIENTS; ++j)
                    clients[j]->send("hello\r\n");
                for (size_t j = 0; j < N_CLIENTS; ++j) {
                    std::string response = clients[j]->recv_spinlock(7, 1000ms).value_or("");
                    CHECK(response == "hello\r\n");
                }
            });
        }
        for (size_t i = 0; i < THREADS; ++i)
            tc[i].join();

        server_running = false;
        t.join();
    }
}