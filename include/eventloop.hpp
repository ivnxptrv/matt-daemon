#pragma once
#include "stream.hpp"
#include <vector>

class Eventloop {
  private:
    int epoll_fd_;
    std::vector<Stream *> srcs_;

  public:
    Eventloop();
    ~Eventloop();
    void run();
    void addEventSource(Stream *s);
    int getSrcsSize();
};

#define MAX_EVENTS 10
