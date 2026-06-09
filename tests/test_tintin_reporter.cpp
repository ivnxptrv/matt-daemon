// Integration tests for Tintin_reporter — exercises real file I/O in /tmp.
// No root needed.
#include "doctest.h"
#include "tintin_reporter.hpp"

#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>

namespace {

std::string read_all(const std::string &path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Unique path per call so tests don't collide.
std::string tmp_log_path() {
    static int counter = 0;
    return "/tmp/matt_daemon_test_log_" + std::to_string(::getpid()) + "_" +
           std::to_string(++counter) + ".log";
}

} // namespace

TEST_CASE("Tintin_reporter: writes one INFO line with timestamp + level + msg") {
    auto path = tmp_log_path();
    {
        Tintin_reporter r(path);
        r.log(Tintin_reporter::Level::INFO, "Matt_daemon: Started.");
    } // dtor flushes/closes

    std::string contents = read_all(path);
    // [DD/MM/YYYY-HH:MM:SS] [ INFO ] - Matt_daemon: Started.\n
    std::regex expected(
        R"(\[\d{2}/\d{2}/\d{4}-\d{2}:\d{2}:\d{2}\] \[ INFO \] - Matt_daemon: Started\.\n)");
    CHECK(std::regex_match(contents, expected));
    ::unlink(path.c_str());
}

TEST_CASE("Tintin_reporter: emits LOG and ERROR levels with correct bracket spacing") {
    auto path = tmp_log_path();
    {
        Tintin_reporter r(path);
        r.log(Tintin_reporter::Level::LOG, "Matt_daemon: User input: hi");
        r.log(Tintin_reporter::Level::ERROR, "Matt_daemon: bad.");
    }
    std::string contents = read_all(path);
    CHECK(contents.find("[ LOG ] - Matt_daemon: User input: hi") !=
          std::string::npos);
    CHECK(contents.find("[ ERROR ] - Matt_daemon: bad.") != std::string::npos);
    ::unlink(path.c_str());
}

TEST_CASE("Tintin_reporter: appends across separate instances (ios::app)") {
    auto path = tmp_log_path();
    { Tintin_reporter(path).log(Tintin_reporter::Level::INFO, "first"); }
    { Tintin_reporter(path).log(Tintin_reporter::Level::INFO, "second"); }

    std::string contents = read_all(path);
    CHECK(contents.find("first") != std::string::npos);
    CHECK(contents.find("second") != std::string::npos);
    // And there should be at least two newlines (one per entry).
    CHECK(std::count(contents.begin(), contents.end(), '\n') == 2);
    ::unlink(path.c_str());
}

TEST_CASE("Tintin_reporter: each log call flushes (unitbuf invariant)") {
    auto path = tmp_log_path();
    Tintin_reporter r(path);
    r.log(Tintin_reporter::Level::INFO, "flushed?");
    // No destructor yet — but the file should already contain the line.
    std::string contents = read_all(path);
    CHECK(contents.find("flushed?") != std::string::npos);
    ::unlink(path.c_str());
}

TEST_CASE("Tintin_reporter: throws when the log path is unopenable") {
    // Parent dir doesn't exist and mkdir of single level can't create the chain.
    CHECK_THROWS_AS(Tintin_reporter("/no/such/path/file.log"),
                    std::runtime_error);
}

TEST_CASE("Tintin_reporter: timestamp is within a few seconds of now") {
    auto path = tmp_log_path();
    std::time_t before = std::time(nullptr);
    {
        Tintin_reporter r(path);
        r.log(Tintin_reporter::Level::INFO, "now");
    }
    std::time_t after = std::time(nullptr);

    std::string contents = read_all(path);
    std::smatch m;
    std::regex ts_re(R"(\[(\d{2})/(\d{2})/(\d{4})-(\d{2}):(\d{2}):(\d{2})\])");
    REQUIRE(std::regex_search(contents, m, ts_re));

    std::tm tm_buf{};
    tm_buf.tm_mday = std::stoi(m[1]);
    tm_buf.tm_mon = std::stoi(m[2]) - 1;
    tm_buf.tm_year = std::stoi(m[3]) - 1900;
    tm_buf.tm_hour = std::stoi(m[4]);
    tm_buf.tm_min = std::stoi(m[5]);
    tm_buf.tm_sec = std::stoi(m[6]);
    tm_buf.tm_isdst = -1;
    std::time_t logged = std::mktime(&tm_buf);

    // Allow ±2 seconds of skew.
    CHECK(logged >= before - 2);
    CHECK(logged <= after + 2);
    ::unlink(path.c_str());
}
