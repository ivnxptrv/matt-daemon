#pragma once
#include "stream.hpp"
#include <string>

class Client : public Stream {
  private:
    std::string buf_;

  public:
    Client(int fd);
    ~Client();
    void handle(Eventloop &el, int event);
};
