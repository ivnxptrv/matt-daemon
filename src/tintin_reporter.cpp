#include "tintin_reporter.hpp"
#include <ctime>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>

Tintin_reporter::Tintin_reporter(const std::string &path) : filepath_(path) {
    // Best-effort: create /var/log/matt_daemon/ if it doesn't exist.
    // EEXIST is fine; any other error will surface when we try to open the
    // file.
    size_t slash = path.rfind('/');
    if (slash != std::string::npos && slash > 0)
        ::mkdir(path.substr(0, slash).c_str(), 0755);

    out_.open(path, std::ios::app);
    if (!out_.is_open())
        throw std::runtime_error("Cannot open log file: " + path);
    // Flush after every entry — guarantees the last line survives a crash/kill.
    // unutbuf permanently changes ofstream behavior to not use buffer and flush
    // instantly
    out_ << std::unitbuf;
}

Tintin_reporter::~Tintin_reporter() {}

void Tintin_reporter::log(Level level, const std::string &msg) {
    out_ << timestamp() << " [ " << level_str(level) << " ] - " << msg << '\n';
}

// returns current time in pritty format string
std::string Tintin_reporter::timestamp() {
    // raw num of sec from 1970
    std::time_t now = std::time(nullptr);
    // human friendy type with years, month etc
    std::tm tm_buf;
    // init tm_buf with raw num of sec
    ::localtime_r(&now, &tm_buf); // reentrant, no shared static buffer
    // buf to store final string
    char buf[32];
    // init buf from tm_buf type
    std::strftime(buf, sizeof(buf), "[%d/%m/%Y-%H:%M:%S]", &tm_buf);
    return buf;
}

const char *Tintin_reporter::level_str(Level l) {
    switch (l) {
    case Level::INFO:
        return "INFO";
    case Level::LOG:
        return "LOG";
    case Level::ERROR:
        return "ERROR";
    }
    return "????";
}
