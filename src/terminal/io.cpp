#include "io.hpp"

#include <fcntl.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#include <windows.h>
#endif

#include <atomic>
#include <cstdio>
#include <system_error>

// #include <fmt/base.h>

#include "terminal/tty.hpp"

namespace terminal {
constinit FILE *file{nullptr};

void init_buff(bool bypass) {
#ifndef _WIN32
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
// fmt::println(stderr, "init buff");
#else
  HANDLE handle = nullptr;
  if (!bypass) {
    handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE || handle == NULL /* NOLINT */) {
      throw std::system_error(static_cast<int>(::GetLastError()),
                              std::system_category(),
                              "(init_buff) failed to get output handle");
    }
  } else {
    handle = termout_handle.load(std::memory_order_relaxed);
    if (handle == nullptr) {
      throw std::runtime_error(
          "(init_buff) terminal ouptut handle isn't here?");
    }
  }

  HANDLE handle_target = nullptr;
  if (DuplicateHandle(::GetCurrentProcess(), handle, ::GetCurrentProcess(),
                      &handle_target, 0, FALSE, DUPLICATE_SAME_ACCESS) == 0) {
    throw std::system_error(static_cast<int>(::GetLastError()),
                            std::system_category(),
                            "(init_buff) failed to duplicate output handle");
  }

  int fd = ::_open_osfhandle(reinterpret_cast<std::intptr_t>(handle_target),
                             _O_TEXT | _O_WRONLY);
  if (fd == -1) {
    ::CloseHandle(handle_target);
    throw std::runtime_error(
        "(init_buff) failed to convert handle to file descriptor");
  }

  file = _fdopen(fd, "w");
  if (file == nullptr) {
    _close(fd);
    throw std::runtime_error(
        "(init_buff) failed to convert file descriptor to FILE*");
  }
#endif
}

void deinit_buff() {
  if (file != nullptr) {
    // fmt::println(stderr, "fclose !");
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
