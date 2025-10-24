#include "server_thread.hh"

#include "server.hh"

ServerThread::ServerThread(class Server *server)
    : server_(server)
{
}

ServerThread::~ServerThread()
{
}

void ServerThread::take_socket_ownership(std::unique_ptr<Socket> socket)
{
}

void ServerThread::remove_socket(SOCKET fd)
{
}

void ServerThread::new_data_ready(SOCKET fd)
{
}

/*
std::string request = recv(*session);
std::string response = session->new_data(request);
kthreadpool_.add_task(fd, [this, &session, response]() { send(*session, response); });  // TODO - send partial data for large blocks
 */
