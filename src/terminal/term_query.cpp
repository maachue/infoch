#include "term_query.hpp"

#ifndef _WIN32
#include <cerrno>
#include <poll.h>
#include <unistd.h>
#else
#include <windows.h>
#include <winternl.h>
#endif

#include <atomic>
#include <cstdarg>
#include <cstdio>
#ifndef _WIN32
#include <cstdlib>
#endif
#include <system_error>

#include "terminal/cbreak_mode.hpp"
#include "terminal/tty.hpp"

namespace terminal {
QueryTerminalErrCategory const &queryterminalerr_category() noexcept {
  static QueryTerminalErrCategory cate;
  return cate;
}

int query_terminal(std::error_code &err, std::string_view query, int nparams,
                   const char *pattern, ...) {
  err.clear();
  if (!terminal::is_init) {
    return -1;
  }

#ifndef _WIN32
  auto n = write(ftty, query.data(), query.length());
  if (n == -1) {
    err = std::error_code(errno, std::generic_category());
    return -1;
  }
  if (static_cast<size_t>(n) != query.length()) {
    err = QueryTerminalErrCode::CannotWriteToTTY;
    return -1;
  }

  n = 0;
  struct pollfd a{.fd = ftty, .events = POLLIN};
  n = poll(&a, 1, 100);
  if (n == -1) {
    err = std::error_code(errno, std::generic_category());
    return -1;
  }
  if (n == 0) {
    err = QueryTerminalErrCode::CannotPollingTTY;
    return -1;
  }

  char buffer[1024];
  size_t bytes_read = 0;
  va_list args;
  va_start(args, pattern);
  n = -1;

  while (true) {
    n = read(STDIN_FILENO, buffer + bytes_read,
             sizeof(buffer) - bytes_read - 1);

    if (n <= 0) {
      va_end(args);

      err = QueryTerminalErrCode::CannotParseInput;
      return -1;
    }

    bytes_read += n;
    buffer[bytes_read] = '\0';

    va_list cargs;
    va_copy(cargs, args);
    int ret = vsscanf(buffer, pattern, cargs);
    va_end(cargs);

    if (ret <= 0) {
      va_end(args);

      err = QueryTerminalErrCode::CannotParseInput;
      return -1;
    }
    if (ret >= nparams) {
      break;
    }
  }

  va_end(args);
#else
  HANDLE htermin = termin_handle.load(std::memory_order_relaxed);
  HANDLE htermout = termout_handle.load(std::memory_order_relaxed);

  {
    DWORD bytes = 0;
    if (WriteFile(htermout, query.data(), query.length(), &bytes, nullptr) ==
        0) {
      err = std::error_code(static_cast<int>(GetLastError()),
                            std::system_category());
      return -1;
    }

    if (bytes != query.length()) {
      err = QueryTerminalErrCode::CannotWriteToTTY;
      return -1;
    }
  }

  LARGE_INTEGER tmp{.QuadPart = (int64_t)100 * -10000};
  while (true) {
    if (NtWaitForSingleObject(htermin, FALSE, &tmp) != STATUS_WAIT_0) {
      err = QueryTerminalErrCode::CannotPollingTTY;
      return -1;
    }

    INPUT_RECORD record{};
    DWORD len = 0;
    if (PeekConsoleInputW(htermin, &record, 1, &len) == 0) {
      break;
    }

    if (record.EventType == KEY_EVENT &&
        record.Event.KeyEvent.uChar.UnicodeChar != L'\r' &&
        record.Event.KeyEvent.uChar.UnicodeChar != L'\n') {
      break;
    }

    ReadConsoleInputW(htermin, &record, 1, &len);
  }

  va_list args;
  va_start(args, pattern);

  char buffer[1024];
  std::uint32_t bytes_read = 0;
  while (true) {
    DWORD bytes = 0;
    if (ReadFile(htermin, buffer + bytes_read,
                 (DWORD)(sizeof(buffer) - 1 - bytes_read), &bytes,
                 nullptr) == 0) {
      va_end(args);
      err = std::error_code(static_cast<int>(GetLastError()),
                            std::system_category());
      return -1;
    }

    if (bytes == 0) {
      va_end(args);
      err = std::error_code(static_cast<int>(GetLastError()),
                            std::system_category());
      return -1;
    }

    bytes_read += bytes;
    if (bytes_read >= sizeof(buffer) - 1) {
      va_end(args);
      err = QueryTerminalErrCode::CannotParseInput;
      return -1;
    }

    buffer[bytes_read] = '\0';
    va_list cargs;
    va_copy(cargs, args);
    int ret = vsscanf(buffer, pattern, cargs);
    va_end(cargs);

    if (ret <= 0) {
      va_end(args);
      err = QueryTerminalErrCode::CannotParseInput;
      return -1;
    }

    if (ret >= nparams) {
      break;
    }
  }

  va_end(args);
#endif
  return 0;
}
} // namespace terminal
