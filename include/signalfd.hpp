#pragma once
#include "stream.hpp"

class Eventloop;
class Tintin_reporter;

class Signalfd : public Stream {
  public:
    Signalfd() = delete;
    Signalfd(const Signalfd &) = delete;
    Signalfd &operator=(const Signalfd &) = delete;

    explicit Signalfd(Tintin_reporter &reporter);
    ~Signalfd();
    void handle(Eventloop &el, int event);

  private:
    Tintin_reporter &reporter_;
};
