#include "eventloop.hpp"
#include "client.hpp"
#include "stream.hpp"
#include <algorithm>
#include <cerrno>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

Eventloop::Eventloop() {
    this->epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (this->epoll_fd_ < 0)
        throw std::runtime_error("epoll_create1 failed");
}

Eventloop::~Eventloop() {
    // Drain by popping back to avoid iterator invalidation:
    // deleteEventSource erases from srcs_ inside the loop.
    while (!srcs_.empty()) {
        Stream *s = srcs_.back();
        this->deleteEventSource(s);
    }
    if (this->epoll_fd_ >= 0)
        ::close(this->epoll_fd_);
}

void Eventloop::shutdown() { shutdown_flag_ = true; }

void Eventloop::addEventSource(Client *s) {
    this->numActiveClients_++;
    this->addEventSource(static_cast<Stream *>(s));
}

void Eventloop::addEventSource(Stream *s) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = s;

    if (::epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, s->getFd(), &ev) < 0) {
        delete s;
        throw std::runtime_error("epoll_ctl ADD failed");
    }
    this->srcs_.push_back(s);
}

void Eventloop::deleteEventSource(Client *s) {
    this->numActiveClients_--;
    this->deleteEventSource(static_cast<Stream *>(s));
}

void Eventloop::deleteEventSource(Stream *s) {
    auto it = std::find(this->srcs_.begin(), this->srcs_.end(), s);
    if (it == this->srcs_.end())
        return;

    ::epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, s->getFd(), nullptr);
    this->srcs_.erase(it);
    delete s;
}

void Eventloop::run() {
    struct epoll_event events[MAX_EVENTS];

    while (!shutdown_flag_) {
        int nfds = ::epoll_wait(this->epoll_fd_, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            // means the system call was interrupted by a signal before it could
            // complete its task
            if (errno == EINTR)
                continue;
            throw std::runtime_error("epoll_wait failed");
        }
        for (int i = 0; i < nfds; i++) {
            static_cast<Stream *>(events[i].data.ptr)
                ->handle(*this, events[i].events);
            if (shutdown_flag_)
                break;
        }
    }
}

int Eventloop::getSrcsSize() { return this->srcs_.size(); }
int Eventloop::getNumActiveClients() { return this->numActiveClients_; }
