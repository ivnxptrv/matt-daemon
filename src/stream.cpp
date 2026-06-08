#include "stream.hpp"

Stream::Stream() {}
Stream::~Stream() {}

int Stream::getFd() const { return this->fd_; }
