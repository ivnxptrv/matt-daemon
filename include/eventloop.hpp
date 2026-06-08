#pragma once
#include "stream.hpp"

class Eventloop {
  private:
    int epoll_fd_;

  public:
    Eventloop();
    ~Eventloop();
    void run();
    void addEventSource(const Stream &s) const;
};

#define MAX_EVENTS 10
