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
    /*
    SessionHash* conn_hash = MALLOC(sizeof *conn_hash);
    conn_hash->fd = client_fd;
    conn_hash->session = server->create_session_cb(client_fd, server->session_data);
    HASH_ADD_INT(server->session_hash, fd, conn_hash);
     */
    // TODO

    // add session to connection pool
    // spool_add_session(server->spool, conn_hash->session);
    // TODO
}

void Server::handle_client_data_ready(SOCKET fd)
{

}

void Server::handle_client_disconnected(SOCKET fd)
{
    printf("New client disconnected!\n");
}
