#pragma once
#include "stream.hpp"

class Listener : public Stream {
  private:
    int numActiveClients_ = 0;

  public:
    Listener() = delete;
    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;

    Listener(int port);
    ~Listener();
    void handle(Eventloop &el, int event);
};
