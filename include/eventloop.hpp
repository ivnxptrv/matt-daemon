#pragma once
#include "stream.hpp"
#include <vector>

class Client;

class Eventloop {
  private:
    int epoll_fd_;
    std::vector<Stream *> srcs_;
    int numActiveClients_ = 0;
    bool shutdown_flag_ = false;

  public:
    Eventloop();
    ~Eventloop();
    Eventloop(const Eventloop &) = delete;
    Eventloop &operator=(const Eventloop &) = delete;

    void run();
    void shutdown();
    void addEventSource(Stream *s);
    void addEventSource(Client *s);
    void deleteEventSource(Stream *s);
    void deleteEventSource(Client *s);
    int getSrcsSize();
    int getNumActiveClients();
};

#define MAX_EVENTS 10
