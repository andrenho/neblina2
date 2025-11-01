#ifndef NEBLINA_TCPCONNECTION_HH
#define NEBLINA_TCPCONNECTION_HH

#include "../connection.hh"

class TCPConnection : public Connection {
public:
    using Connection::Connection;

    [[nodiscard]] std::string recv() const override;
    void                      send(std::string const &data) const override;
};


#endif //NEBLINA_TCPCONNECTION_HH
