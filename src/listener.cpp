#include "listener.hpp"
#include "client.hpp"
#include "eventloop.hpp"
#include "tintin_reporter.hpp"
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Listener::Listener(int port, Tintin_reporter &reporter) : reporter_(reporter) {
    this->fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd_ < 0)
        throw std::runtime_error("Failed to create socket");

    int opt = 1;
    // to be able fastly restart on same port
    ::setsockopt(this->fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = ::htons(static_cast<uint16_t>(port));

    if (::bind(this->fd_, reinterpret_cast<struct sockaddr *>(&addr),
               sizeof(addr)) < 0) {
        ::close(this->fd_);
        this->fd_ = -1;
        throw std::runtime_error("Failed to bind socket");
    }
    // Backlog = 3 matches subject's max concurrent clients; SOMAXCONN works
    // too but is misleading given the cap.
    if (::listen(this->fd_, 3) < 0) {
        ::close(this->fd_);
        this->fd_ = -1;
        throw std::runtime_error("Failed to listen");
    }
}

Listener::~Listener() {}

void Listener::handle(Eventloop &el, int event) {
    if (!(event & EPOLLIN))
        return;

    int fd = ::accept(this->fd_, nullptr, nullptr);
    if (fd < 0)
        return;

    // 3-client cap: close immediately if at limit so the kernel sends FIN.
    if (el.getNumActiveClients() >= 3) {
        ::close(fd);
        return;
    }
    el.addEventSource(new Client(fd, reporter_));
}
