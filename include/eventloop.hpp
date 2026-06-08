#pragma once
#include "client.hpp"
#include "stream.hpp"
#include <vector>

class Eventloop {
  private:
    int epoll_fd_;
    std::vector<Stream *> srcs_;
    int numActiveClients_ = 0;

  public:
    Eventloop();
    ~Eventloop();
    void run();
    void addEventSource(Stream *s);
    void addEventSource(Client *s);
    void deleteEventSource(Stream *s);
    void deleteEventSource(Client *s);
    int getSrcsSize();
    int getNumActiveClients();
};

#define MAX_EVENTS 10
