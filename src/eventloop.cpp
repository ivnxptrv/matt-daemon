#include "eventloop.hpp"
#include "stream.hpp"
#include <algorithm>
#include <iostream>
#include <sys/epoll.h>

Eventloop::Eventloop() { this->epoll_fd_ = epoll_create1(0); }

Eventloop::~Eventloop() {
    // TOOD: close epoll fd
    for (Stream *s : srcs_) {
        delete s; // Clean up memory for each pointer
    }
    srcs_.clear();
}

void Eventloop::addEventSource(Stream *s) {

    struct epoll_event ev;

    this->srcs_.push_back(s);

    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = s;
    epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, s->getFd(), &ev);
}

void Eventloop::deleteEventSource(Stream *s) {
    auto it = std::find(this->srcs_.begin(), this->srcs_.end(), s);

    if (it != this->srcs_.end()) {
        epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, s->getFd(), nullptr);
        this->srcs_.erase(it);
        delete s;
    }
}

void Eventloop::run() {

    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int nfds = epoll_wait(this->epoll_fd_, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            ((Stream *)events[0].data.ptr)->handle(*this, events[0].events);
            std::cout << "here: " << this->getSrcsSize() << std::endl;
        }
    }
}

int Eventloop::getSrcsSize() { return this->srcs_.size(); }
