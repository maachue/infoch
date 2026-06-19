#include "cbreak_mode.hpp"

#ifndef __unix__
#error                                                                         \
    "Sorry, current project only supports for Unix (Linux, MacOS, FreeBSD, ...)"
#endif

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <system_error>

#include <fmt/core.h>

#include "terminal/tty.hpp"

namespace terminal {
constinit std::atomic<bool> is_init{false};
static struct termios original_term{};

bool init_cbreak_mode() {
  auto fd = ftty.load(std::memory_order_relaxed);
  bool is_init_ = fd != -1;

  if (is_init_) {
    if (tcgetattr(fd, &original_term) == -1) {
      throw std::system_error(errno, std::generic_category(),
                              "failed to get configuration of current tty");
    }

    struct termios edit = original_term;

    edit.c_lflag &= ~(ICANON | ECHO);

    // edit.c_cc[VMIN] = 0;
    // edit.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &edit) == -1) {
      throw std::system_error(errno, std::generic_category(),
                              "failed to get configuration of current tty");
    }
  }

  is_init.store(is_init_);
  fmt::println(stderr, "init cbreak");

  return is_init;
}

void deinit_cbreak_mode() noexcept {
  int fd = ftty.load();
  if (fd == -1) {
    fmt::println(
        stderr,
        "/dev/tty must not close when call deinit_mode! Something went wrong!");
    std::abort();
  }

  if (is_init.load(std::memory_order_relaxed)) {
    is_init = tcsetattr(fd, TCSANOW, &original_term) == -1;
  }
}
} // namespace terminal
