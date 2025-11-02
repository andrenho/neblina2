#include "doctest.h"

#include <atomic>
#include <memory>
#include <thread>

#include "neblina.hh"

#define PORT 23456

class EchoProtocol : public Protocol {
public:
    class Session : public ::Session {
    public:
        explicit Session(std::unique_ptr<Connection> connection) : ::Session(std::move(connection)) {}

    protected:
        std::string process(std::string const &data) override {
            return data;
        }
    };

    [[nodiscard]] std::unique_ptr<::Session> create_session(std::unique_ptr<Connection> connection) const override {
        return std::make_unique<Session>(std::move(connection));
    }
};

TEST_SUITE("TCP Server")
{
    TEST_CASE("Single-threaded server and client")
    {
        logging_verbose = true;
        logging_dest = stderr;

        TCPServer server(PORT, false, std::make_unique<EchoProtocol>(), Threads::Single);
        TCPClient client1("127.0.0.1", PORT);
        TCPClient client2("127.0.0.1", PORT);

        client1.send("hello\r\n");
        client2.send("hellw\r\n");
        std::this_thread::sleep_for(6ms);

        for (size_t i = 0; i < 10; ++i)
            server.iterate();

        std::string response = client1.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "hello\r\n");

        response = client2.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "hellw\r\n");
    }

    TEST_CASE("Multi-threaded server and single-threaded client")
    {
        logging_verbose = true;
        logging_dest = stderr;

        TCPServer server(PORT, false, std::make_unique<EchoProtocol>(), 2);
        TCPClient client1("127.0.0.1", PORT);
        TCPClient client2("127.0.0.1", PORT);
        TCPClient client3("127.0.0.1", PORT);

        client1.send("hello\r\n");
        client2.send("hellw\r\n");
        client3.send("helly\r\n");
        std::this_thread::sleep_for(100ms);

        for (size_t i = 0; i < 10; ++i)
            server.iterate();

        std::string response = client1.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "hello\r\n");

        response = client2.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "hellw\r\n");

        response = client3.recv_spinlock(7, 1000ms).value_or("");
        CHECK(response == "helly\r\n");
    }
}