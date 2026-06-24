#include "term_size_query.hpp"

#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "terminal/term_query.hpp"

namespace terminal {
void query_terminal_termsize_pixel(std::uint16_t &width,
                                   std::uint16_t &height) {
  auto params = query_terminal_ii<1024, "\x1b[14t", "\x1b[4;", 't', "termsize",
                                  "query_terminal_termsize_pixel">();

  if (params.size() <= 1 || params.size() > 2) {
    throw std::runtime_error(
        fmt::format("(query_terminal_termsize_pixel) invalid params while "
                    "query terminal size in pixels: {}",
                    params));
  }

  width = params[1];
  height = params[0];
}

void query_terminal_termsize_cell(std::uint16_t &width, std::uint16_t &height) {
  auto params = query_terminal_ii<1024, "\x1b[18t", "\x1b[8;", 't', "termsize",
                                  "query_terminal_termsize_cell">();

  if (params.size() <= 1 || params.size() > 2) {
    throw std::runtime_error(
        fmt::format("(query_terminal_termsize_cell) invalid params while "
                    "query terminal size in pixels: {}",
                    params));
  }

  width = params[1];
  height = params[0];
}
} // namespace terminal