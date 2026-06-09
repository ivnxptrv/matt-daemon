#include "signalfd.hpp"
#include "eventloop.hpp"
#include "tintin_reporter.hpp"
#include <signal.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

Signalfd::Signalfd(Tintin_reporter &reporter) : reporter_(reporter) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGQUIT);

    // Order matters: block default delivery BEFORE creating the signalfd.
    // Otherwise an in-flight signal can kill us between the two calls.
    if (::sigprocmask(SIG_BLOCK, &mask, nullptr) < 0)
        throw std::runtime_error("sigprocmask failed");

    this->fd_ = ::signalfd(-1, &mask, SFD_CLOEXEC);
    if (this->fd_ < 0)
        throw std::runtime_error("signalfd creation failed");
}

Signalfd::~Signalfd() {}

void Signalfd::handle(Eventloop &el, int event) {
    if (!(event & EPOLLIN)) return;

    struct signalfd_siginfo si;
    ssize_t n = ::read(this->fd_, &si, sizeof(si));
    if (n != sizeof(si)) return;

    reporter_.log(Tintin_reporter::Level::INFO, "Matt_daemon: Signal handler.");
    reporter_.log(Tintin_reporter::Level::INFO, "Matt_daemon: Quitting.");
    el.shutdown();
}
