#include "server.hh"

#include <csignal>

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
    static Server* global_server = this;
    signal(SIGINT, [](int) {
        if (global_server) {
            printf("CTRL+C detected - finalizing server...\n");
            global_server->stop();
        }
    });
}