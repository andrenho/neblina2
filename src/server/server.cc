#include "server.hh"

Server::Server(std::unique_ptr<Protocol> protocol, std::unique_ptr<Socket> socket, size_t n_threads)
    : protocol_(std::move(protocol)), server_socket_(std::move(socket)), kthreadpool_(n_threads), poller_(*server_socket_)
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
                handle_client_data_ready(event.fd);
                break;
            case Poller::EventType::ClientDisconnected:
                handle_client_disconnected(event.fd);
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

    // add session to connection pool
    // spool_add_session(server->spool, conn_hash->session);
    // TODO
}

void Server::handle_client_data_ready(SOCKET fd)
{

}

void Server::handle_client_disconnected(SOCKET fd)
{
    DBG("Client disconnected from socket {}", fd);

    // find connection
    auto it = sessions_.find(fd);
    if (it == sessions_.end())
        return;
    auto const& session = it->second;

    client_disconnected(session->socket());

    // remove connection from connection pool
    // TODO
    // spool_remove_session(server->spool, conn_hash->session);

    // remove socket from poller
    poller_.remove_client(&session->socket());

    // remove session from list and close socket
    sessions_.erase(it);
}
