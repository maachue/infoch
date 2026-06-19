#include "io.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <system_error>

#include <fmt/base.h>

#include "terminal/tty.hpp"

namespace terminal {
constinit FILE *file{nullptr};

void init_buff(bool bypass) {
  int stdout_fd = -1;
  if (!bypass) {
    stdout_fd = dup(STDOUT_FILENO);
  } else {
    stdout_fd = dup(ftty.load(std::memory_order_relaxed));
  }
  if (stdout_fd == -1) {
    throw fmt::system_error(errno,
                            "(init_buff) cannot duplicate STDOUT_FILENO or "
                            "controlling tty (fd = {})",
                            ftty.load(std::memory_order_relaxed));
  }

  int flags = fcntl(stdout_fd, F_GETFD);
  if (flags == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "(init_buff) cannot get flags from STDOUT_FILENO "
                            "or contrlling tty file descriptor");
  }
  if (fcntl(stdout_fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        "(init_buff) cannot add \"close on exec\" to duplicate STDOUT_FILENO "
        "or controlling tty file descriptor");
  }

  file = fdopen(stdout_fd, "w");
  if (file == nullptr) {
    close(stdout_fd);
    throw std::system_error(
        errno, std::generic_category(),
        "(init_buff) cannot convert file descriptor to FILE*");
  }
  fmt::println(stderr, "init buff");
}

void deinit_buff() {
  if (file != nullptr) {
    fclose(file);
    file = nullptr;
  }
}

void flush() { std::fflush(file); }

void vprint(fmt::string_view fmt, fmt::format_args args) {
  fmt::vprint(file, fmt, args);
}

void vprintln(fmt::string_view fmt, fmt::format_args args) {
  fmt::vprintln(file, fmt, args);
}
} // namespace terminal
