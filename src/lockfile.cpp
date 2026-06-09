#include "lockfile.hpp"
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/file.h>
#include <unistd.h>

LockFile::LockFile(const std::string &path) : fd_(-1), path_(path) {
    fd_ = ::open(path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd_ < 0) {
        std::cerr << "Can't open :" << path << std::endl;
        throw std::runtime_error("lockfile: open failed");
    }
    // LOCK_EX | LOCK_NB: subject forbids LOCK_SH; non-blocking so a second
    // instance fails fast instead of waiting.
    if (::flock(fd_, LOCK_EX | LOCK_NB) < 0) {
        std::cerr << "Can't open :" << path << std::endl;
        ::close(fd_);
        fd_ = -1;
        throw std::runtime_error("lockfile: already locked");
    }
}

LockFile::~LockFile() {
    if (fd_ < 0) return;
    ::flock(fd_, LOCK_UN);
    ::close(fd_);
    ::unlink(path_.c_str());
}
