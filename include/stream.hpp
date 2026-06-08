#pragma once

class Stream {
  protected:
    int fd_ = -1;

  public:
    Stream();
    ~Stream();
    int getFd() const;
    virtual void handle(int event) = 0;
};
