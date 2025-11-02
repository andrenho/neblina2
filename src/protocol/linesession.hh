#ifndef NEBLINA_LINESESSION_HH
#define NEBLINA_LINESESSION_HH

#include "session.hh"

class LineSession : public Session {
protected:
    explicit LineSession(std::unique_ptr<Connection> connection, std::string const& separator="\r\n", bool include_separator=true)
        : Session(std::move(connection)), separator_(separator), separator_len_(separator.length()), include_separator_(include_separator) {}

    void iteration() override;

private:
    std::string separator_;
    size_t      separator_len_;
    bool        include_separator_;
    std::string buffer_;
};


#endif //NEBLINA_LINESESSION_HH
