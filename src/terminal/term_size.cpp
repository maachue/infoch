#include "term_size.hpp"

#include <sys/ioctl.h>

#include <atomic>
#include <system_error>

#include "terminal/term_query.hpp"
#include "terminal/tty.hpp"

namespace terminal {
constinit TermSize termsize{};

const std::error_category &termfetchsize_category() noexcept {
  static TermFetchSizeErrCategory cate;
  return cate;
}

int fetch_terminal_size(std::error_code &err) noexcept {
  err.clear();
  struct winsize winsize{};

  if (ioctl(ftty.load(std::memory_order_relaxed), TIOCGWINSZ, &winsize) < 0) {
    err = std::error_code(errno, std::generic_category());
    return -1;
  }

  if (winsize.ws_col == 0 || winsize.ws_row == 0) {
    auto errn = query_terminal(err, "\x1b[18t", 2, "\x1b[8;%hu;%hut",
                               &winsize.ws_row, &winsize.ws_col);

    if (errn != 0) {
      if (err != QueryTerminalErrCode::CannotParseInput) {
        return -1;
      }
    }
  }

  if (winsize.ws_xpixel == 0 || winsize.ws_ypixel == 0) {
    auto errn = query_terminal(err, "\x1b[14t", 2, "\x1b[4;%hu;%hut",
                               &winsize.ws_ypixel, &winsize.ws_xpixel);

    if (errn != 0) {
      if (err != QueryTerminalErrCode::CannotParseInput) {
        return -1;
      }

      err = TermFetchSizeErrCode::CannotQueryPixelSize;
    }
  }

  if (winsize.ws_col == 0 || winsize.ws_row == 0) {
    err = TermFetchSizeErrCode::CannotQueryCellSize;
    return -1;
  }

  termsize = TermSize{.cell_width = winsize.ws_col,
                      .cell_height = winsize.ws_row,
                      .pixel_width = winsize.ws_xpixel,
                      .pixel_height = winsize.ws_ypixel};
  return 0;
}
} // namespace terminal
