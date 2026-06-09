#include "stream.hpp"

Stream::Stream() : Fd() {}
Stream::Stream(int fd) : Fd(fd) {}

// Empty: Fd owns the descriptor and closes it.
Stream::~Stream() {}
