#include "daemonize.hpp"
#include "eventloop.hpp"
#include "listener.hpp"
#include "lockfile.hpp"
#include "signalfd.hpp"
#include "tintin_reporter.hpp"

#include <iostream>
#include <string>
#include <unistd.h>

static const char *kLockPath = "/var/lock/matt_daemon.lock";
static const char *kLogPath = "/var/log/matt_daemon/matt_daemon.log";
static const int kPort = 4242;

int main() {
    if (::geteuid() != 0) {
        std::cerr << "Matt_daemon: must run as root." << std::endl;
        return 1;
    }

    try {
        // Order matters. Lock first while stderr is still attached so the
        // "Can't open :..." message reaches the user. Then open the log.
        LockFile lock(kLockPath);
        Tintin_reporter reporter(kLogPath);

        reporter.log(Tintin_reporter::Level::INFO, "Matt_daemon: Started.");
        reporter.log(Tintin_reporter::Level::INFO,
                     "Matt_daemon: Creating server.");

        Eventloop el;
        el.addEventSource(new Listener(kPort, reporter));

        reporter.log(Tintin_reporter::Level::INFO,
                     "Matt_daemon: Server created.");
        reporter.log(Tintin_reporter::Level::INFO,
                     "Matt_daemon: Entering Daemon mode.");

        // After this returns we're the grandchild — no tty, no stdio.
        // File descriptors (lock, log, listening socket, epoll) are inherited.
        daemonize();

        reporter.log(Tintin_reporter::Level::INFO,
                     "Matt_daemon: started. PID: " +
                         std::to_string(::getpid()) + ".");

        // Set up signalfd in the final process so sigprocmask applies here,
        // not in the parents that have already exited.
        el.addEventSource(new Signalfd(reporter));

        el.run();
        // "Quitting." is logged by whichever handler triggered the shutdown
        // (Signalfd or Client on "quit"), matching the subject's example.
    } catch (const std::exception &) {
        // LockFile already wrote the user-visible error to stderr if that
        // was the failure. Anything else fails silently after detach.
        return 1;
    }
    return 0;
}
