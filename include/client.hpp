#pragma once
#include "stream.hpp"

class Client : public Stream {
  public:
    Client(int fd);
    ~Client();
    void handle(Eventloop &el, int event);
};
