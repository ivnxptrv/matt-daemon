#include "client.hpp"
#include "eventloop.hpp"
#include "tintin_reporter.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>

Client::Client(int fd, Tintin_reporter &reporter)
    : Stream(fd), reporter_(reporter) {}

Client::~Client() {}

void Client::handle(Eventloop &el, int event) {
    // EPOLLRDHUP: Peer closed the connection gracefully (stopped sending data).
    // EPOLLHUP: The connection was broken or hung up unexpectedly.
    // EPOLLERR: A hard error occurred on the socket; it is now unusable.
    if (event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        el.deleteEventSource(this);
        return;
    }
    if (!(event & EPOLLIN))
        return;

    char chunk[4096];
    ssize_t n = ::recv(this->fd_, chunk, sizeof(chunk), 0);
    if (n <= 0) {
        // 0 = peer closed; <0 = error. Either way, drop the client.
        el.deleteEventSource(this);
        return;
    }

    this->buf_.append(chunk, n);

    // TCP is a byte stream — extract every complete line in the buffer.
    size_t pos;
    while ((pos = this->buf_.find('\n')) != std::string::npos) {
        std::string line = this->buf_.substr(0, pos);
        this->buf_.erase(0, pos + 1);
        if (!line.empty() && line.back() == '\r') // telnet sends CRLF
            line.pop_back();

        if (line == "quit") {
            reporter_.log(Tintin_reporter::Level::INFO,
                          "Matt_daemon: Request quit.");
            reporter_.log(Tintin_reporter::Level::INFO,
                          "Matt_daemon: Quitting.");
            el.shutdown();
            return;
        }
        reporter_.log(Tintin_reporter::Level::LOG,
                      "Matt_daemon: User input: " + line);
    }
}
