#include "server.hh"

Server::Server(std::unique_ptr<Protocol> protocol, Socket socket, size_t n_threads)
    : protocol_(std::move(protocol)), server_socket_(std::move(socket)), kthreadpool_(n_threads), poller_(server_socket_)
{

}

void Server::iterate()
{
    for (Poller::Event const& event: poller_.wait()) {
        switch (event.type) {
            case Poller::EventType::NewConnection:
                break;
            case Poller::EventType::DataReady:
                break;
            case Poller::EventType::Disconnected:
                break;
        }
    }
}

void Server::run()
{
    while (running_)
        iterate();
}
