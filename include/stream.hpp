#pragma once
#include "fd.hpp"

class Eventloop;

class Stream : public Fd {
  public:
    Stream();
    explicit Stream(int fd);
    virtual ~Stream();
    Stream(const Stream &) = delete;
    Stream &operator=(const Stream &) = delete;
    virtual void handle(Eventloop &el, int event) = 0;
};
