#include "echo.hh"

std::unique_ptr<Session> EchoProtocol::create_session(std::unique_ptr<Socket> socket) const
{
    return std::make_unique<Session>(std::move(socket));  // TODO - custom session
}
