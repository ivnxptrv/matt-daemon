// Integration tests for LockFile — exercises real open + flock + unlink in /tmp.
// No root needed.
#include "doctest.h"
#include "lockfile.hpp"

#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace {
std::string tmp_lock_path() {
    static int counter = 0;
    return "/tmp/matt_daemon_test_lock_" + std::to_string(::getpid()) + "_" +
           std::to_string(++counter) + ".lock";
}

bool exists(const std::string &path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}
} // namespace

TEST_CASE("LockFile: creates the file on construction") {
    auto path = tmp_lock_path();
    {
        LockFile l(path);
        CHECK(exists(path));
    }
    // After destruction, the file must be unlinked so the next instance can start.
    CHECK_FALSE(exists(path));
}

TEST_CASE("LockFile: a second LockFile on the same path throws") {
    auto path = tmp_lock_path();
    LockFile first(path);
    CHECK_THROWS_AS(LockFile second(path), std::runtime_error);
}

TEST_CASE("LockFile: after destruction, the lock can be re-acquired") {
    auto path = tmp_lock_path();
    { LockFile l(path); }
    // No throw expected.
    LockFile l2(path);
    CHECK(exists(path));
}

TEST_CASE("LockFile: throws when the path is unwritable") {
    // / is owned by root and not writable as the test user — open() should fail.
    CHECK_THROWS_AS(LockFile l("/this_should_fail_at_top.lock"),
                    std::runtime_error);
}
