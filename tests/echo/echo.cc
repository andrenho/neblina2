#include "echo.hh"

std::unique_ptr<Session> EchoProtocol::create_session(std::unique_ptr<Socket> socket) const
{
    return std::make_unique<EchoSession>(std::move(socket));  // TODO - custom session
}

std::string EchoSession::new_data(std::string const& data)
{
    return data;
}
