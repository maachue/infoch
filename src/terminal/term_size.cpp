#include "term_size.hpp"

#ifndef _WIN32
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

#include <atomic>
#include <exception>
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
  try {
#ifndef _WIN32
    struct winsize winsize{};

    if (ioctl(ftty.load(std::memory_order_relaxed), TIOCGWINSZ, &winsize) < 0) {
      throw std::system_error(errno, std::generic_category(),
                              "(fetch_terminal_size) something went wrong");
    }

    if (winsize.ws_col == 0 || winsize.ws_row == 0) {
      query_terminal_termsize_cell(termsize.cell_width, termsize.cell_height);
    }

    if (winsize.ws_xpixel == 0 || winsize.ws_ypixel == 0) {
      query_terminal_termsize_pixel(termsize.pixel_width,
                                    termsize.pixel_height);
    }

    if (winsize.ws_col == 0 || winsize.ws_row == 0) {
      throw std::system_error(TermFetchSizeErrCode::CannotQueryCellSize,
                              "(fetch_terminal_size) cannot get terminal size");
    }

    termsize = TermSize{.cell_width = winsize.ws_col,
                        .cell_height = winsize.ws_row,
                        .pixel_width = winsize.ws_xpixel,
                        .pixel_height = winsize.ws_ypixel};
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
      query_terminal_termsize_pixel(termsize.pixel_width,
                                    termsize.pixel_height);
    }

    if (termsize.is_zero()) {
      throw std::system_error(TermFetchSizeErrCode::CannotQueryCellSize,
                              "(fetch_terminal_size) cannot get terminal size");
    }
#endif
  } catch (...) {
    std::throw_with_nested(std::runtime_error("failed to fetch terminal size"));
  }
}
} // namespace terminal
