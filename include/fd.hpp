#pragma once

class Fd {
  protected:
    int fd_ = -1;

  public:
    Fd();
    explicit Fd(int fd);
    virtual ~Fd();
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;
    int getFd() const;
};
