#ifndef INFOCH_TERMINAL_TERM_SIZE_H
#define INFOCH_TERMINAL_TERM_SIZE_H

#include <cstdint>

namespace terminal {
struct TermSize {
  std::uint16_t cell_width;
  std::uint16_t cell_height;

  std::uint16_t pixel_width;
  std::uint16_t pixel_height;

  [[nodiscard]] bool is_zero() const noexcept {
    return cell_height == 0 || cell_height == 0 || pixel_height == 0 ||
           pixel_width == 0;
  }
};

extern constinit TermSize termsize;

void fetch_terminal_size();
} // namespace terminal

#endif
