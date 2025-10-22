#ifndef NEBLINA_ECHO_PROTOCOL_HH
#define NEBLINA_ECHO_PROTOCOL_HH

#include "protocol/protocol.hh"
#include "protocol/split_session.hh"

class EchoSession : public SplitSession {   // TODO - use LineSession
public:
    using SplitSession::SplitSession;

    std::string new_strings(const std::vector<std::string> &strs) override;
};

class EchoProtocol : public Protocol {
public:
    [[nodiscard]] std::unique_ptr<Session> create_session(std::unique_ptr<Socket> socket) const override;
};

#endif //NEBLINA_ECHO_PROTOCOL_HH
