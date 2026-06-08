#pragma once
#include "stream.hpp"

class Listener : public Stream {
  public:
    Listener() = delete;
    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;

    Listener(int port);
    ~Listener();
    void handle(int event);
};
