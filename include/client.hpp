#pragma once
#include "line_framer.hpp"
#include "stream.hpp"

class Tintin_reporter;

class Client : public Stream {
  public:
    Client() = delete;
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(int fd, Tintin_reporter &reporter);
    ~Client();
    void handle(Eventloop &el, int event);

  private:
    LineFramer framer_;
    Tintin_reporter &reporter_;
};
