#include "echo.hh"

#include <string_view>
#include <ranges>

std::unique_ptr<Session> EchoProtocol::create_session(std::unique_ptr<Socket> socket) const
{
    return std::make_unique<EchoSession>(std::move(socket));  // TODO - custom session
}

std::string EchoSession::new_strings(std::vector<std::string> const& strs)
{
    std::string r;
    for (auto const& s: strs)
        r += s;
    return r;
}
