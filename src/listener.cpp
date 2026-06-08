#include "listener.hpp"
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Listener::Listener(int port) { // Initialize with -1
    this->fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    ::setsockopt(this->fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = ::htons(static_cast<uint16_t>(port));

    if (::bind(this->fd_, reinterpret_cast<struct sockaddr *>(&addr),
               sizeof(addr)) < 0) {
        ::close(this->fd_);
        throw std::runtime_error("Failed to bind socket");
    }

    if (::listen(this->fd_, SOMAXCONN) < 0) {
        ::close(this->fd_);
        throw std::runtime_error("Failed to listen");
    }
}

Listener::~Listener() {
    if (this->fd_ >= 0) {
        ::close(this->fd_);
    }
}

void Listener::handle(int event) {
    if (event == EPOLLIN) {
        ::accept(this->fd_, nullptr, nullptr);
    }
}
