// Unit tests for LineFramer — pure logic, no syscalls.
#include "doctest.h"
#include "line_framer.hpp"

TEST_CASE("LineFramer: one full line in one feed") {
    LineFramer f;
    auto out = f.feed("hello\n", 6);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == "hello");
}

TEST_CASE("LineFramer: partial line buffered until newline arrives") {
    LineFramer f;
    CHECK(f.feed("hel", 3).empty());
    CHECK(f.feed("lo", 2).empty());
    auto out = f.feed("\n", 1);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == "hello");
}

TEST_CASE("LineFramer: multiple lines in one feed") {
    LineFramer f;
    auto out = f.feed("a\nb\nc\n", 6);
    REQUIRE(out.size() == 3);
    CHECK(out[0] == "a");
    CHECK(out[1] == "b");
    CHECK(out[2] == "c");
}

TEST_CASE("LineFramer: CR stripped before LF (CRLF clients)") {
    LineFramer f;
    auto out = f.feed("hello\r\n", 7);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == "hello");
}

TEST_CASE("LineFramer: bare LF (no CR) preserved as-is") {
    LineFramer f;
    auto out = f.feed("hello\n", 6);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == "hello"); // not "hell" — only \r before \n is stripped
}

TEST_CASE("LineFramer: fragmented quit detected (byte-by-byte)") {
    // The 42-evaluator-classic test in unit form.
    LineFramer f;
    CHECK(f.feed("q", 1).empty());
    CHECK(f.feed("u", 1).empty());
    CHECK(f.feed("i", 1).empty());
    auto out = f.feed("t\n", 2);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == "quit");
}

TEST_CASE("LineFramer: empty line preserved") {
    LineFramer f;
    auto out = f.feed("\n", 1);
    REQUIRE(out.size() == 1);
    CHECK(out[0].empty());
}

TEST_CASE("LineFramer: bytes after final newline stay buffered for next call") {
    LineFramer f;
    auto a = f.feed("done\nmore", 9);
    REQUIRE(a.size() == 1);
    CHECK(a[0] == "done");

    auto b = f.feed("!\n", 2);
    REQUIRE(b.size() == 1);
    CHECK(b[0] == "more!");
}

TEST_CASE("LineFramer: embedded NUL bytes don't truncate the line") {
    LineFramer f;
    const char data[] = {'a', '\0', 'b', '\n'};
    auto out = f.feed(data, 4);
    REQUIRE(out.size() == 1);
    REQUIRE(out[0].size() == 3); // 'a', '\0', 'b'
    CHECK(out[0][0] == 'a');
    CHECK(out[0][1] == '\0');
    CHECK(out[0][2] == 'b');
}

TEST_CASE("LineFramer: feed of zero bytes is a no-op") {
    LineFramer f;
    CHECK(f.feed("anything", 0).empty());
    auto out = f.feed("x\n", 2);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == "x");
}
