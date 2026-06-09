#include "fd.hpp"
#include <unistd.h>

Fd::Fd() : fd_(-1) {}
Fd::Fd(int fd) : fd_(fd) {}

Fd::~Fd() {
    if (fd_ >= 0)
        ::close(fd_);
}

int Fd::getFd() const { return fd_; }
