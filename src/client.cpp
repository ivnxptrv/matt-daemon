#include "client.hpp"
#include "eventloop.hpp"
#include "tintin_reporter.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>

Client::Client(int fd, Tintin_reporter &reporter)
    : Stream(fd), reporter_(reporter) {}

Client::~Client() {}

void Client::handle(Eventloop &el, int event) {
    // EPOLLRDHUP: peer closed gracefully. EPOLLHUP: hang up. EPOLLERR: hard error.
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

    // Framing is delegated to LineFramer (pure logic, unit-tested separately).
    for (const auto &line : framer_.feed(chunk, n)) {
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
