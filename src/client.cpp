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
        char buffer[1024];
        ssize_t bytes_read = ::recv(this->fd_, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes_read] = '\0';
        std::cout << "bytes: " << bytes_read << buffer << std::endl;
        // TODO: handle long chunks?
    }
}
