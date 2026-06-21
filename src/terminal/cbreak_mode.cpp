#include "cbreak_mode.hpp"
#include <stdexcept>

#ifndef _WIN32
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <atomic>
#include <system_error>

#include <fmt/core.h>

#include "terminal/tty.hpp"

namespace terminal {
constinit std::atomic<bool> is_init{false};
#ifndef _WIN32
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
  // fmt::println(stderr, "init cbreak");

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
    // fmt::println(stderr, "deinit cbreak");
    is_init = tcsetattr(fd, TCSANOW, &original_term) == -1;
  }
}
#else

static DWORD dword_termout{};
static DWORD dword_termin{};

bool init_cbreak_mode() {
  auto *termout_h = termout_handle.load(std::memory_order_relaxed);
  auto *termin_h = termin_handle.load(std::memory_order_relaxed);
  bool is_init_ = termout_h != nullptr && termin_h != nullptr;
  BOOL err{};

  if (is_init_) {
    if (GetConsoleMode(termin_h, &dword_termin) == 0) {
      throw std::system_error(
          (int)GetLastError(), std::system_category(),
          "(init_cbreak_mode) failed to get DWORD of terminal input");
    }
    if (GetConsoleMode(termout_h, &dword_termout) == 0) {
      throw std::system_error(
          (int)GetLastError(), std::system_category(),
          "(init_cbreak_mode) failed to get DWORD of terminal output");
    }

    auto termin_ = dword_termin;
    auto termout_ = dword_termout;

    termin_ &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    termin_ |= ENABLE_VIRTUAL_TERMINAL_INPUT;

    termout_ |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (SetConsoleMode(termin_h, termin_) == 0) {
      throw std::system_error(
          (int)GetLastError(), std::system_category(),
          "(init_cbreak_mode) failed to set DWORD of terminal input");
    }

    if (SetConsoleMode(termout_h, termout_) == 0) {
      SetConsoleMode(termin_h, dword_termin);
      throw std::system_error(
          (int)GetLastError(), std::system_category(),
          "(init_cbreak_mode) failed to set DWORD of terminal output");
    }
  }

  is_init.store(is_init_);

  return is_init;
}

void deinit_cbreak_mode() noexcept {
  if (is_init) {
    auto *termout_h = termout_handle.load(std::memory_order_relaxed);
    auto *termin_h = termin_handle.load(std::memory_order_relaxed);
    BOOL err{};
    BOOL err1{};
    if (termout_h != nullptr) {
      err = SetConsoleMode(termout_h, dword_termout);
    }

    if (termin_h != nullptr) {
      err1 = SetConsoleMode(termin_h, dword_termin);
    }

    is_init.store(err == 0 || err1 == 0);
  }
}
#endif
} // namespace terminal
