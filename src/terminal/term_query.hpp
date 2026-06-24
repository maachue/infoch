#ifndef INFOCH_TERMINAL_TERM_QUERY_H
#define INFOCH_TERMINAL_TERM_QUERY_H

#ifndef _WIN32
#include <cerrno>
#include <poll.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winternl.h>
#endif

#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstdint>
#include <ranges>
#include <system_error>
#include <vector>

#include <fmt/format.h>

#include "terminal/cbreak_mode.hpp"
#include "terminal/tty.hpp"

namespace terminal {
template <size_t N> struct QueryBuffer {
  static constexpr size_t actual_size = (N > 0) ? (N - 1) : 0;
  char buf[actual_size > 0 ? actual_size : 1];

  constexpr QueryBuffer(const char (&buff)[N]) {
    if constexpr (actual_size > 0) {
      std::copy_n(buff, actual_size, buf);
    } else {
      buf[0] = '\0';
    }
  }

  [[nodiscard]] constexpr size_t size() const { return actual_size; }
  [[nodiscard]] constexpr size_t length() const { return actual_size; }

  [[nodiscard]] constexpr std::string_view to_string_view() const {
    return {buf, actual_size};
  }
};

template <QueryBuffer Query, const char kEnd, QueryBuffer ErrorPattern = "",
          QueryBuffer kWhere = "query_terminal__", size_t N>
[[nodiscard]] size_t query_terminal_i(char (&buffer)[N]) {
  size_t bytes_read = 0;

  if (!is_init.load(std::memory_order_relaxed)) {
    throw std::runtime_error(fmt::format("({}) terminal isn't in cbreak mode!",
                                         kWhere.to_string_view()));
  }

#ifndef _WIN32
  auto devtty = ftty.load(std::memory_order_relaxed);
  ssize_t n = 0;
  n = write(devtty, Query.buf, Query.length());
  if (n == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("({}) failed to write escape sequence {} to /dev/tty",
                    kWhere.to_string_view(), ErrorPattern.to_string_view()));
  }

  if (n != Query.length()) {
    throw std::runtime_error(
        fmt::format("({}) failed to write escape "
                    "sequence {} to /dev/tty: not enough bytes",
                    kWhere.to_string_view(), ErrorPattern.to_string_view()));
  }

  struct pollfd a{.fd = ftty, .events = POLLIN};
  n = poll(&a, 1, 100);
  if (n == -1) {
    throw std::system_error(errno, std::generic_category(),
                            fmt::format("({}) failed to polling /dev/tty",
                                        kWhere.to_string_view()));
  }
  if (n == 0) {
    throw std::runtime_error(fmt::format(
        "({}) failed to polling /dev/tty: timeout?", kWhere.to_string_view()));
  }

  while (true) {
    n = read(ftty, buffer + bytes_read, N - bytes_read);

    if (n < 0) {
      throw std::system_error(
          errno, std::generic_category(),
          fmt::format("({}) failed to read /dev/tty", kWhere.to_string_view()));
    }

    if (n == 0) {
      break;
    }

    bytes_read += n;

    if (bytes_read >= N) {
      bytes_read = bytes_read - N == 0 ? N : bytes_read - N;
      break;
    }

    if (buffer[bytes_read - 1] == kEnd) {
      break;
    }
  }
#else
  auto *htermout = termout_handle.load(std::memory_order_relaxed);
  auto *htermin = termin_handle.load(std::memory_order_relaxed);

  DWORD n = 0;
  if (WriteFile(htermout, Query.buf, Query.length(), &n, nullptr) == 0) {
    throw std::system_error(
        static_cast<int>(GetLastError()), std::system_category(),
        fmt::format("({}) failed to write escape "
                    "sequence {} to terminal output handle",
                    kWhere.to_string_view(), ErrorPattern.to_string_view()));
  }

  if (n != Query.length()) {
    throw std::runtime_error(fmt::format(
        "({}) failed to write escape sequence {} to terminal output "
        "handle: not enough bytes",
        kWhere.to_string_view(), ErrorPattern.to_string_view()));
  }

  LARGE_INTEGER tmp{.QuadPart = (int64_t)100 * -10000};
  while (true) {
    if (NtWaitForSingleObject(htermin, FALSE, &tmp) != STATUS_WAIT_0) {
      throw std::runtime_error(fmt::format(
          "({}) failed to polling terminal input", kWhere.to_string_view()));
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
    if (ReadFile(htermin, buffer + bytes_read, N - bytes_read, &n, nullptr) ==
        0) {
      throw std::system_error(
          static_cast<int>(GetLastError()), std::system_category(),
          fmt::format("({}) failed to read terminal input handle",
                      kWhere.to_string_view()));
    }

    if (n == 0) {
      break;
    }

    bytes_read += n;

    if (bytes_read >= N) {
      bytes_read = bytes_read - N == 0 ? N : bytes_read - N;
      break;
    }

    if (buffer[bytes_read - 1] == kEnd) {
      break;
    }
  }
#endif

  return bytes_read;
}

template <size_t N, QueryBuffer Query, QueryBuffer Begin, const char End,
          QueryBuffer ErrorPattern = "",
          QueryBuffer Where = "query_terminal_ii">
std::vector<std::uint16_t> query_terminal_ii() {
  char buffer[N];

  size_t bytes_read =
      query_terminal_i<Query, End, ErrorPattern, Where, N>(buffer);

  std::string_view buff(buffer, bytes_read);
  std::vector<std::uint16_t> result;

  size_t start_pos = buff.find(Begin.to_string_view());
  if (start_pos == std::string_view::npos) {
    throw std::runtime_error(
        fmt::format("({}) cannot find start escape sequence from terminal "
                    "replied",
                    Where.to_string_view()));
  }

  start_pos += Begin.size();

  size_t end_pos = buff.find(End, start_pos);
  if (end_pos == std::string_view::npos) {
    throw std::runtime_error(fmt::format(
        "({}) cannot find end of escape sequence from terminal replied: end={}",
        Where.to_string_view(), End));
  }

  std::string_view token{};
  for (auto &&subrange : buff | std::ranges::views::split(';')) {
    std::string_view token(subrange.begin(), subrange.end());

    if (token.empty()) {
      result.emplace_back(-1);
    }

    std::uint16_t val = 0;

    auto [ptr, ec] =
        std::from_chars(token.data(), token.data() + token.size(), val);

    if (ec == std::errc{}) {
      result.emplace_back(val);
    }
  }

  return result;
}
} // namespace terminal

#endif
