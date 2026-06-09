#include "daemonize.hpp"
#include "lockfile.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

void daemonize(LockFile &lock) {
    pid_t pid = ::fork();
    if (pid < 0)
        throw std::runtime_error("fork 1 failed");
    if (pid > 0)
        ::_exit(0); // parent leaves without running stack dtors

    if (::setsid() < 0)
        throw std::runtime_error("setsid failed");

    // Second fork: child is no longer a session leader, so it can never
    // reacquire a controlling terminal even if it opens a tty later.
    pid = ::fork();
    if (pid < 0)
        throw std::runtime_error("fork 2 failed");
    if (pid > 0)
        ::_exit(0);

    if (::chdir("/") < 0)
        throw std::runtime_error("chdir failed");
    // default umask (often set to 0022) automatically strips away certain bits,
    // so we ensure there is no mask
    ::umask(0);

    // Detach stdio: redirect 0/1/2 to /dev/null so any stray write
    // doesn't error or end up on a random fd.
    ::close(STDIN_FILENO);
    ::close(STDOUT_FILENO);
    ::close(STDERR_FILENO);
    int dn = ::open("/dev/null", O_RDWR);
    if (dn >= 0) {
        ::dup2(dn, STDIN_FILENO);
        ::dup2(dn, STDOUT_FILENO);
        ::dup2(dn, STDERR_FILENO);
        if (dn > 2)
            ::close(dn);
    }
    lock.writePid();
}
