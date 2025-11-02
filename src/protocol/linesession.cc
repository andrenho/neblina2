#include "linesession.hh"

#include <vector>

void LineSession::iteration()
{
    std::string response;
    buffer_ += connection_->recv();

    std::vector<std::string> strs;
    size_t i = 0;
    for (;;) {
        i = buffer_.find(separator_, i);
        if (i != std::string::npos) {
            response += process(buffer_.substr(0, i + separator_len_));
            buffer_ = buffer_.erase(0, i + separator_len_);
        } else {
            break;
        }
    }

    connection_->send(response);
}
