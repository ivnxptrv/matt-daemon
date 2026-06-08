#include "client.hpp"
#include "eventloop.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>

Client::Client(int fd) { this->fd_ = fd; }

Client ::~Client() {}

void Client::handle(Eventloop &el, int event) {
    if (event & EPOLLRDHUP) {
        std::cout << "bye" << std::endl;
        el.deleteEventSource(this);
        return;
    }
    if (event & EPOLLIN) {
        // move chars from socket to buffer
        char buffer[1024];
        ssize_t bytes_read = ::recv(this->fd_, buffer, sizeof(buffer), 0);

        // add to clients buffer, to be preserved
        this->buf_.append(buffer, bytes_read);

        // search for a done new-line terminated line in the buffer
        size_t pos;
        while ((pos = this->buf_.find('\n')) != std::string::npos) {
            std::string line = this->buf_.substr(0, pos);

            // delete found line form the client's buffer
            this->buf_.erase(0, pos + 1);

            std::cout << "line: " << line << std::endl;
        }
    }
}
