#include "client.hpp"
#include "eventloop.hpp"

Client::Client(int fd) { this->fd_ = fd; }

Client ::~Client() {}

void Client::handle(Eventloop &el, int event) { (void)event, (void)el; }
