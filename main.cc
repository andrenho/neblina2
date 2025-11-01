#include "neblina.hh"

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

int main()
{
    TCPServer server(23456, false, std::make_unique<EchoProtocol>(), Threads::Single);
    server.run();
}