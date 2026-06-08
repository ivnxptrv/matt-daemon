#pragma once

class Eventloop;

class Stream {
  protected:
    int fd_ = -1;

  public:
    Stream();
    virtual ~Stream();
    int getFd() const;
    virtual void handle(Eventloop &el, int event) = 0;
};
