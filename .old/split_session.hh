#ifndef NEBLINA_SPLIT_SESSION_HH
#define NEBLINA_SPLIT_SESSION_HH

#include <vector>

#include "session.hh"

class SplitSession : public Session {
public:
    explicit SplitSession(std::unique_ptr<Socket> socket, std::string const& separator="\r\n")
        : Session(std::move(socket)), separator_(separator), separator_len_(separator.length()) {}

    virtual std::string new_strings(std::vector<std::string> const& strs) = 0;

    std::string new_data(std::string const& data) override {
        buffer_ += data;

        std::vector<std::string> strs;
        size_t i = 0;
        for (;;) {
            i = buffer_.find(separator_, i);
            if (i != std::string::npos) {
                strs.emplace_back(buffer_.substr(0, i + separator_len_));
                buffer_ = buffer_.erase(0, i + separator_len_);
            } else {
                if (strs.empty())
                    return {};
                return new_strings(strs);
            }
        }
    }

private:
    const size_t separator_len_;
    const std::string separator_;
    std::string buffer_;
};


#endif //NEBLINA_SPLIT_SESSION_HH
