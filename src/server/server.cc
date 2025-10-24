#include "server.hh"

Server::Server(std::unique_ptr<Protocol> protocol, std::unique_ptr<Socket> socket, size_t n_threads)
    : protocol_(std::move(protocol)), server_socket_(std::move(socket)), poller_(server_socket_->fd)
{
    for (size_t i = 0; i < n_threads; ++i)
        server_threads_.push_back(std::make_unique<ServerThread>(this));
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
                server_threads_.at(thread_hash(event.fd))->new_data_ready(event.fd);
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

    // pass socket ownership to thread
    SOCKET fd = client->fd;
    server_threads_.at(thread_hash(client->fd))->take_socket_ownership(std::move(client));

    // add socket to poller
    poller_.add_client(fd);
}
