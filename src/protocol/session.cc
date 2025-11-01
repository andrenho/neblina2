#include "session.hh"

void Session::iteration()
{
    std::string request = connection_->recv();
    if (!request.empty()) {
        std::string response = process(request);
        connection_->send(response);
    }
}
