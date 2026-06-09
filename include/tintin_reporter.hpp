#pragma once
#include <fstream>
#include <string>

class Tintin_reporter {
  public:
    enum class Level { INFO, LOG, ERROR };

    Tintin_reporter() = delete;
    Tintin_reporter(const Tintin_reporter &) = delete;
    Tintin_reporter &operator=(const Tintin_reporter &) = delete;

    explicit Tintin_reporter(const std::string &path);
    ~Tintin_reporter();

    void log(Level level, const std::string &msg);

  private:
    // output file stream, automatically close fd
    std::ofstream out_;
    const std::string filepath_;

    static std::string timestamp();
    static const char *level_str(Level l);
};
