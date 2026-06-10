#include "line_framer.hpp"

std::vector<std::string> LineFramer::feed(const char *data, std::size_t len) {
    buf_.append(data, len);

    std::vector<std::string> out;
    std::size_t pos;
    while ((pos = buf_.find('\n')) != std::string::npos) {
        std::string line = buf_.substr(0, pos);
        buf_.erase(0, pos + 1);
        if (!line.empty() && line.back() == '\r') // CRLF clients
            line.pop_back();
        out.push_back(std::move(line));
    }
    return out; // return empty 0 vector if no \n found
}
