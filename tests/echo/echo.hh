#ifndef NEBLINA_ECHO_PROTOCOL_HH
#define NEBLINA_ECHO_PROTOCOL_HH

#include "protocol/protocol.hh"

class EchoSession : public Session {   // TODO - use LineSession
public:
    using Session::Session;

    std::string new_data(std::string const& data) override;
};

class EchoProtocol : public Protocol {
public:
    [[nodiscard]] std::unique_ptr<Session> create_session(std::unique_ptr<Socket> socket) const override;
};

#endif //NEBLINA_ECHO_PROTOCOL_HH
