#include "server.hh"

#include "util/log.hh"

Server::Server(std::unique_ptr<Protocol> protocol, std::unique_ptr<Socket> socket, ThreadCount n_threads)
    : protocol_(std::move(protocol)), server_socket_(std::move(socket)), poller_(server_socket_->fd)
{
    if (size_t const* n = std::get_if<size_t>(&n_threads)) {
        for (size_t i = 0; i < *n; ++i)
            server_threads_.push_back(std::make_unique<ServerThread>(*this, i));
    } else {
        server_threads_.push_back(std::make_unique<ServerThread>(*this, 0, false));
    }

    for (auto& thread: server_threads_)
        thread->start([](std::string const& error) { ERR("{}", error); });  // TODO - deal with errors
}

void Server::finalize()
{
    for (auto& thread: server_threads_)
        thread->stop();
    server_threads_.clear();
}

size_t Server::thread_hash(SOCKET fd) const
{
    return fd % this->server_threads_.size();
}

void Server::iterate()
{
    for (Poller::Event const& event: poller_.wait()) {

        switch (event.type) {
            case Poller::EventType::NewClient:
                handle_new_client();
                break;
            case Poller::EventType::ClientDataReady:
                server_threads_.at(thread_hash(event.fd))->push(event.fd);
                break;
            case Poller::EventType::ClientDisconnected:
                DBG("Client disconnected from socket {}", event.fd);
                server_threads_.at(thread_hash(event.fd))->remove_socket(event.fd);
                break;
        }

    }
}

void Server::run()
{
    while (running_)
        iterate();
}

void Server::handle_new_client()
{
    // accept new connection
    std::unique_ptr<Socket> client = accept_new_connection();
    SOCKET fd = client->fd;

    DBG("server: new client detected (socket {})", fd);

    // create session and pass ownership to thread
    auto session = protocol_->create_session(std::move(client));
    server_threads_.at(thread_hash(fd))->add_session(std::move(session));

    // add socket to poller
    poller_.add_client(fd);
}
