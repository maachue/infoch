#include "cbreak_mode.hpp"

#ifndef __unix__
#error                                                                         \
    "Sorry, current project only supports for Unix (Linux, MacOS, FreeBSD, ...)"
#endif

#include <cerrno>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <system_error>

#include "terminal/types.hpp"

namespace terminal {
std::atomic<INFOCH_TERMINAL_STREAM> ftty{-1};
std::atomic<bool> is_init{};
struct termios original_term{};

bool init_cbreak_mode(std::error_code &err) noexcept {
  err.clear();
  ftty = open("/dev/tty", O_CLOEXEC | O_RDWR | O_NOCTTY);
  is_init = ftty != -1;

  if (is_init) {
    if (tcgetattr(ftty, &original_term) == -1) {
      err = std::error_code(errno, std::generic_category());
      is_init = false;
      return is_init;
    }

    struct termios edit = original_term;

    edit.c_lflag &= ~(ICANON | ECHO);

    // edit.c_cc[VMIN] = 0;
    // edit.c_cc[VTIME] = 0;

    setbuf(stdin, nullptr); // NOLINT

    is_init = tcsetattr(ftty, TCSANOW, &edit) != -1;
  }

  if (!is_init) {
    err = std::error_code(errno, std::generic_category());
  }

  return is_init;
}

void deinit_mode() noexcept {
  if (is_init) {
    is_init = tcsetattr(ftty, TCSANOW, &original_term) == -1;
    close(ftty);
  }
}
} // namespace terminal
