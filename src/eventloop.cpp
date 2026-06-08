#include "eventloop.hpp"
#include "stream.hpp"
#include <iostream>
#include <sys/epoll.h>

Eventloop::Eventloop() { this->epoll_fd_ = epoll_create1(0); }

Eventloop::~Eventloop() {
    // TOOD: close epoll fd
}

void Eventloop::addEventSource(const Stream &s) const {

    struct epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.ptr = (void *)&s;
    epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, s.getFd(), &ev);
}

void Eventloop::run() {

    struct epoll_event events[MAX_EVENTS];

    while (true) {

        int nfds = epoll_wait(this->epoll_fd_, events, MAX_EVENTS, -1);
        (void)nfds;
        ((Stream *)events[0].data.ptr)->handle(events[0].events);
        std::cout << "here" << std::endl;
    }
}
