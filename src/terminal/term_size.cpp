#include "term_size.hpp"

#ifndef _WIN32
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

#include <atomic>
#include <system_error>

#include "terminal/term_size_query.hpp"
#include "terminal/tty.hpp"

namespace terminal {
constinit TermSize termsize{};

const std::error_category &termfetchsize_category() noexcept {
  static TermFetchSizeErrCategory cate;
  return cate;
}

void fetch_terminal_size() {
#ifndef _WIN32
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

      err = TermFetchSizeErrCode::CannotQueryPixelSize;
      return -1;
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
      return -1;
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
#else
  auto handle = termout_handle.load(std::memory_order_relaxed);
  CONSOLE_SCREEN_BUFFER_INFO buffsize{};
  auto errn = GetConsoleScreenBufferInfo(handle, &buffsize);
  if (errn == 0) {
    throw std::system_error(static_cast<int>(::GetLastError()),
                            std::system_category(),
                            "(fetch_terminal_size) something went wrong");
  }

  termsize.cell_width = buffsize.srWindow.Right - buffsize.srWindow.Left + 1;
  termsize.cell_height = buffsize.srWindow.Bottom - buffsize.srWindow.Top + 1;
  if (termsize.cell_width == 0 || termsize.cell_height == 0) {
    query_terminal_termsize_cell(termsize.cell_width, termsize.cell_height);
  }

  CONSOLE_FONT_INFOEX font_info{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
  if (GetCurrentConsoleFontEx(handle, FALSE, &font_info) == 0) {
    throw std::system_error(static_cast<int>(::GetLastError()),
                            std::system_category(),
                            "(fetch_terminal_size) something went wrong");
  }

  termsize.pixel_width = termsize.cell_width * font_info.dwFontSize.X;
  termsize.pixel_height = termsize.cell_height * font_info.dwFontSize.Y;
  if (termsize.pixel_width == 0 || termsize.pixel_height == 0) {
    query_terminal_termsize_pixel(termsize.pixel_width, termsize.pixel_height);
  }

  if (termsize.is_zero()) {
    throw std::system_error(
        std::error_code(TermFetchSizeErrCode::CannotQueryCellSize));
  }
#endif
}
} // namespace terminal
