#include "term_query.hpp"

#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
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

  return 0;
}
} // namespace terminal
