#include "server.hh"

void Server::iterate()
{
    for (Poller::Event const& event: poller_.wait()) {

        switch (event.type) {
        case Poller::EventType::NewClient:
            thread_manager_.add_session(protocol_->create_session(create_connection(accept())));
            break;
        case Poller::EventType::ClientDataReady:
            thread_manager_.data_available(event.fd);
            break;
        case Poller::EventType::ClientDisconnected:
            thread_manager_.remove_session(event.fd);
            break;
        }

    }
}