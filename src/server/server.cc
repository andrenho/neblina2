#include "server.hh"

#include <csignal>

Server* Server::global_server_ = nullptr;

void Server::iterate()
{
    for (Poller::Event const& event: poller_.wait()) {

        switch (event.type) {
        case Poller::EventType::NewClient: {
            SOCKET fd = accept();
            thread_manager_.add_session(protocol_->create_session(create_connection(fd)));
            poller_.add_client(fd);
            break;
        }
        case Poller::EventType::ClientDataReady:
            thread_manager_.data_available(event.fd);
            break;
        case Poller::EventType::ClientDisconnected:
            thread_manager_.remove_session(event.fd);
            break;
        }

    }
}

void Server::stop_on_SIGINT()
{
    Server::global_server_ = this;
    signal(SIGINT, [](int) {
        if (Server::global_server_) {
            printf("CTRL+C detected - finalizing server...\n");
            Server::global_server_->stop();
        }
    });
}