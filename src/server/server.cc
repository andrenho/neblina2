#include "server.hh"

Server::Server(std::unique_ptr<Protocol> protocol, std::unique_ptr<Socket> socket, size_t n_threads)
    : protocol_(std::move(protocol)), server_socket_(std::move(socket)), poller_(*server_socket_), kthreadpool_(n_threads)
{

}

void Server::iterate()
{
    for (Poller::Event const& event: poller_.wait()) {

        switch (event.type) {
            case Poller::EventType::NewClient:
                handle_new_client();
                break;
            case Poller::EventType::ClientDataReady:
                kthreadpool_.add_task(event.fd, [this, fd=event.fd]() { handle_client_data_ready(fd); });
                break;
            case Poller::EventType::ClientDisconnected:
                kthreadpool_.add_task(event.fd, [this, fd=event.fd]() { handle_client_disconnected(fd); });
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

    // add socket to poller
    poller_.add_client(client.get());

    // create session and add to map
    SOCKET fd = client->fd;
    std::unique_ptr<Session> session = protocol_->create_session(std::move(client));
    sessions_[fd] = std::move(session);
}

void Server::handle_client_disconnected(SOCKET fd)
{
    DBG("Client disconnected from socket {}", fd);

    // find connection
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;
    auto const& session = it->second;

    // remove socket from poller
    poller_.remove_client(&session->socket());

    // remove session from list and close socket
    sessions_.erase(it);
}

void Server::handle_client_data_ready(SOCKET fd)
{
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;
    auto const& session = it->second;

    std::string request = recv(*session);
    std::string response = session->new_data(request);
    kthreadpool_.add_task(fd, [this, &session, response]() { send(*session, response); });  // TODO - send partial data for large blocks
}
