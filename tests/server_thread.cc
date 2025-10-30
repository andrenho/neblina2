#include "doctest.h"

#include <string>

#include "server/serverthread.hh"

class MyIO : public ISocketIO {
public:
    mutable std::string sent;

    std::string recv(SOCKET fd) const override {
        return "hello" + std::to_string(fd);
    }

    void send(SOCKET fd, std::string const& data) const override {
        sent = data;
    }
};

class MySession : public Session {
public:
    using Session::Session;

    std::string new_data(std::string const& data) override {
        return data + "x";
    }
};

TEST_SUITE("ServerThread") {

    TEST_CASE("Process request") {

        const SOCKET FD = 20;

        MyIO my_io;
        ServerThread server_thread(my_io, 1);
        auto session = std::make_unique<MySession>(std::make_unique<Socket>(FD));

        server_thread.start();
        server_thread.add_session(std::move(session));
        server_thread.push(FD);
        server_thread.stop();

        CHECK(my_io.sent == "hello20x");
    }

}