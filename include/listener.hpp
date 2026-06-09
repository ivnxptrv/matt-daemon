#pragma once
#include "stream.hpp"

class Tintin_reporter;

class Listener : public Stream {
  public:
    Listener() = delete;
    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;

    Listener(int port, Tintin_reporter &reporter);
    ~Listener();
    void handle(Eventloop &el, int event);

  private:
    Tintin_reporter &reporter_;
};
