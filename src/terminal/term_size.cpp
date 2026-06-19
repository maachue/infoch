#include "term_size.hpp"

#include <sys/ioctl.h>

#include <atomic>
#include <stdexcept>
#include <system_error>

#include "terminal/tty.hpp"

namespace terminal {
constinit TermSize termsize{};

void fetch_terminal_size() {
  struct winsize winsize{};

  if (ioctl(ftty.load(std::memory_order_relaxed), TIOCGWINSZ, &winsize) < 0) {
    throw std::system_error(
        errno, std::generic_category(),
        "(fetch_terminal_size) failed to fetch terminal size");
  }

  if (winsize.ws_col == 0 || winsize.ws_row == 0 || winsize.ws_ypixel == 0 ||
      winsize.ws_xpixel == 0) {
    throw std::runtime_error(
        "(fetch_terminal_size) failed to fetch terminal size by unkown error "
        "... Maybe terminal isn't support query termsize via ioctl driver?");
  }

  termsize = TermSize{.cell_width = winsize.ws_col,
                      .cell_height = winsize.ws_row,
                      .pixel_width = winsize.ws_xpixel,
                      .pixel_height = winsize.ws_ypixel};
}
} // namespace terminal
