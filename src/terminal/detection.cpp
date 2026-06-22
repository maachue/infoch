#include "detection.hpp"

#ifndef _WIN32
#include <cerrno>
#include <poll.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winternl.h>
#endif

#include <atomic>
#include <charconv>
#include <ranges>
#include <stdexcept>
#include <system_error>

#include <cstdlib>
#include <cstring>

#include <fmt/base.h>

#include "terminal/tty.hpp"

// cross-platform between windows and Unix
#define WEZTERM_DETECTION                                                      \
  std::getenv("WEZTERM_CONFIG_DIR") || std::getenv("WEZTERM_EXECUTABLE") ||    \
      std::getenv("WEZTERM_PLANE") || std::getenv("WEZTERM_CONFIG_FILE") ||    \
      std::getenv("WEZTERM_EXECUTABLE_DIR") ||                                 \
      std::getenv("WEZTERM_UNIX_SOCKET")

namespace terminal {
bool is_support_sixel() {
  constexpr std::string_view kRequestDA = "\x1b[c";

  char buffer[1024];
  size_t bytes_read = 0;

#ifndef _WIN32
  auto devtty = ftty.load(std::memory_order_relaxed);
  ssize_t n = 0;
  n = write(devtty, kRequestDA.data(), kRequestDA.length());
  if (n == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        "(is_support_sixel) failed to write escape sequence DA (Primary Device "
        "Attribute) to /dev/tty");
  }

  if (n != kRequestDA.length()) {
    throw std::runtime_error(
        "(is_support_sixel) failed to write escape sequence DA (Primary Device "
        "Attribute) to /dev/tty: not enough bytes");
  }

  struct pollfd a{.fd = ftty, .events = POLLIN};
  n = poll(&a, 1, 100);
  if (n == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "(is_support_sixel) failed to polling /dev/tty");
  }
  if (n == 0) {
    throw std::runtime_error(
        "(is_support_sixel) failed to polling /dev/tty: timeout?");
  }

  while (true) {
    n = read(ftty, buffer + bytes_read, sizeof(buffer) - bytes_read);

    if (n < 0) {
      throw std::system_error(errno, std::generic_category(),
                              "(is_support_sixel) failed to read /dev/tty");
    }

    if (n == 0) {
      break;
    }

    bytes_read += n;

    if (bytes_read >= sizeof(buffer) - bytes_read) {
      break;
    }

    if (buffer[bytes_read - 1] == 'c') {
      break;
    }
  }
#else
  auto *htermout = termout_handle.load(std::memory_order_relaxed);
  auto *htermin = termin_handle.load(std::memory_order_relaxed);

  DWORD n = 0;
  if (WriteFile(htermout, kRequestDA.data(), kRequestDA.length(), &n,
                nullptr) == 0) {
    throw std::system_error(
        static_cast<int>(GetLastError()), std::system_category(),
        "(is_support_sixel) failed to write escape sequence DA (Primary Device "
        "Attribute) to terminal output handle");
  }

  if (n != kRequestDA.length()) {
    throw std::runtime_error(
        "(is_support_sixel) failed to write escape sequence DA (Primary Device "
        "Attribute) to terminal output handle: not enough bytes");
  }

  LARGE_INTEGER tmp{.QuadPart = (int64_t)100 * -10000};
  while (true) {
    if (NtWaitForSingleObject(htermin, FALSE, &tmp) != STATUS_WAIT_0) {
      throw std::runtime_error(
          "(is_support_sixel) failed to polling terminal input");
    }

    INPUT_RECORD record{};
    if (PeekConsoleInputW(htermin, &record, 1, &n) == 0) {
      break;
    }

    if (record.EventType == KEY_EVENT &&
        record.Event.KeyEvent.uChar.UnicodeChar != L'\r' &&
        record.Event.KeyEvent.uChar.UnicodeChar != L'\n') {
      break;
    }

    ReadConsoleInputW(htermin, &record, 1, &n);
  }

  while (true) {
    if (ReadFile(htermin, buffer + bytes_read, sizeof(buffer) - bytes_read, &n,
                 nullptr) == 0) {
      throw std::system_error(
          static_cast<int>(GetLastError()), std::system_category(),
          "(is_support_sixel) failed to read terminal input handle");
    }

    if (n == 0) {
      break;
    }

    bytes_read += n;

    if (bytes_read >= sizeof(buffer) - bytes_read) {
      break;
    }

    if (buffer[bytes_read - 1] == 'c') {
      break;
    }
  }
#endif

  std::string_view buff(buffer, bytes_read);
  if (buff.empty()) {
    throw std::runtime_error("(is_support_sixel) failed to know DA (Primary "
                             "Device Attribute): buffer empty");
  }

  size_t start_pos = buff.find("\x1b[?");
  if (start_pos == std::string_view::npos) {
    throw std::runtime_error("(is_support_sixel) failed to know DA (Primary "
                             "Device Attribute): cannot find CSI?");
  }

  start_pos += 2;

  size_t end_pos = buff.find('c', start_pos);
  if (end_pos == std::string_view::npos) {
    throw std::runtime_error(
        "(is_support_sixel) failed to know DA (Primary Device Attribute): "
        "cannot find 'c' end character");
  }

  buff = buff.substr(start_pos, end_pos - start_pos);

  std::string_view token{};
  for (auto &&subrange : buff | std::ranges::views::split(';')) {
    std::string_view token(subrange.begin(), subrange.end());

    if (token.empty()) {
      continue;
      // NOTE: cannot throw error because on some terminal such as kitty, it
      // replied with a parameter actually contains nothing like: \x1b[?62;52;c.

      // throw std::runtime_error("(is_support_sixel) failed to know DA (Primary
      // "
      //                          "Device Attribute): parameters contain
      //                          nothing");
    }

    std::uint16_t val = 0;

    auto [ptr, ec] =
        std::from_chars(token.data(), token.data() + token.size(), val);

    if (ec == std::errc{}) {
      if (val == 4) {
        return true;
      }
    }
  }

  return false;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
const Terminal &get_terminal(bool force_screen, bool force_tmux,
                             bool force_ssh) {
  static Terminal term{};
  static bool is_init{false};
  if (is_init) {
    return term;
  }

  if (force_screen) {
    term.is_screen = true;
  }

  if (force_tmux) {
    term.is_tmux = true;
  }

  if (force_ssh) {
    term.is_ssh = true;
  }

  if (!term.is_ssh) {
    term.is_ssh = std::getenv("SSH_TTY") != nullptr;
  }

  auto *term_env_ = std::getenv("TERM");
  auto *term_program_env_ = std::getenv("TERM_PROGRAM");

  if (!term.is_tmux) {
    term.is_tmux =
        (term_env_ && std::strncmp(term_env_, "tmux", 4) == 0) ||
        (term_program_env_ && std::strncmp(term_program_env_, "tmux", 4) == 0);
  }

  if (!term.is_screen) {
    term.is_screen = (term_env_ && std::strncmp(term_env_, "screen", 6) == 0) ||
                     (term_program_env_ &&
                      std::strncmp(term_program_env_, "screen", 6) == 0);
  }

  if (std::getenv("KITTY_PID") || std::getenv("KITTY_INSTALLATION_DIR")) {
    term.name = "kitty";
  } else if (std::getenv("KONSOLE_VERSION")) {
    term.name = "konsole";
  } // else if (std::getenv("ALACRITTY_LOG") || std::getenv("ALACRITTY_SOCKET")
    // ||
    //          std::getenv("ALACRITTY_WINDOW_ID")) {
    // term.name = "alacritty";
  // }
  else if (std::getenv("GHOSTTY_BIN_DIR") ||
           std::getenv("GHOSTTY_RESOURCES_DIR") ||
           std::getenv("GHOSTTY_SHELL_FEATURES")) {
    term.name = "ghostty";
  } else if (WEZTERM_DETECTION) {
    term.name = "wezterm";
  }
#if defined(__linux__) || defined(_WIN32)
  // WSL and Windows (Windows Terminal)
  else if (std::getenv("WT_SESSION") || std::getenv("WT_PROFILE_ID")) {
    term.name = "windows_terminal";
  }
#endif
  else if (char *lc_term = std::getenv("LC_TERMINAL")) {
    term.name = lc_term;
  } else if (term_program_env_) {
    term.name = term_program_env_;
  } else {
    term.name = term_env_ ? term_env_ : std::string_view{} /* nullptr */;
  }

  if (term.is_ssh) {
    // SSH isn't expose terminal specific env like KITTY_PID

    if (term.name == "Iterm2" /* via LC_TERMINAL */) {
      term.name = "iTerm.app";
    } else if (term.name.contains("kitty") /* via TERM */) {
      term.name = "kitty";
    } else if (term.name.contains("ghostty")) {
      term.name = "ghostty";
    } else if (term.name.contains("WezTerm")) {
      term.name = "wezterm";
    }
  }

  if (term.name == "kitty" || term.name == "konsole" ||
      term.name == "wezterm" || term.name == "ghostty") {
    term.support_kitty = true;
  }

  if (term.name == "wezterm" ||
      term.name == "iTerm.app" /* via TERM_PROGRAM */) {
    term.support_iterm = true;
  }

  if (term.is_tmux /* tmux always expose '4' in \x1b[c although terminal isn't
                      supported */
      || term.name == "wezterm" || term.name == "konsole" ||
      term.name == "foot" || term.name == "windows_terminal") {
    term.support_sixel = true;
  }

  // We already know protocol to use, don't query either
  if (!term.support_sixel && !term.support_iterm && !term.support_kitty) {
    term.support_sixel = is_support_sixel();
  }

  is_init = true;
  return term;
}
} // namespace terminal
