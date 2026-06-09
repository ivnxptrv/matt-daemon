#pragma once
#include <cstddef>
#include <string>
#include <vector>

// Stateful line framer for byte-stream input (e.g. TCP).
// Pure logic, no syscalls — unit-testable in isolation.
class LineFramer {
  public:
    LineFramer() = default;
    LineFramer(const LineFramer &) = default;
    LineFramer &operator=(const LineFramer &) = default;
    ~LineFramer() = default;

    // Append `len` bytes from `data`; return every complete `\n`-terminated
    // line that is now extractable. Trailing CR is stripped. Any bytes after
    // the last `\n` stay buffered for the next call.
    std::vector<std::string> feed(const char *data, std::size_t len);

  private:
    std::string buf_;
};
