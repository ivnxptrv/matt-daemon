#include "stream.hpp"
#include <unistd.h>

Stream::Stream() {}
Stream::~Stream() {
    if (this->fd_ > 0) {
        ::close(this->fd_);
    }
}

int Stream::getFd() const { return this->fd_; }
