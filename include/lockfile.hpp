#pragma once
#include <string>

class LockFile {
  public:
    LockFile() = delete;
    LockFile(const LockFile &) = delete;
    LockFile &operator=(const LockFile &) = delete;

    explicit LockFile(const std::string &path);
    ~LockFile();

  private:
    int fd_;
    const std::string path_;
};
