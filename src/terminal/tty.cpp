#include "tty.hpp"

#include <fcntl.h>

#include <stdexcept>
#include <system_error>

// #include <fmt/base.h>

namespace terminal {
constinit std::atomic<term_t> ftty{-1};

void open_devtty() {
  if (ftty.load() != -1) {
    throw std::runtime_error("(open_devtty) DO NOT CALL THIS FUNCTION TWICE");
  }

  int fd = ::open("/dev/tty", O_CLOEXEC | O_NOCTTY | O_RDWR);
  if (fd == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "failed to open \"/dev/tty\"");
  }
  // fmt::println(stderr, "open /dev/tty");
  ftty.store(fd);
}

void close_devtty() {
  int fd = ftty.load();
  if (fd != -1) {
    // fmt::println(stderr, "close /dev/tty");
    ::close(fd);
    ftty.store(-1); // backup!
  }
}
} // namespace terminal
