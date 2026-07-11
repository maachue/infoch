#include "text.hpp"

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <string_view>
#include <vector>

#include <fmt/printf.h>

#include "terminal/io.hpp"

namespace text {
void print_text(std::string &str, std::uint16_t &x, std::uint16_t &y) {
  std::size_t count_lines = std::count(str.begin(), str.end(), '\n');

  std::vector<std::string_view> lines =
      str | std::views::split('\n') |
      std::views::transform([](auto &&rng) { return std::string_view(rng); }) |
      std::ranges::to<std::vector<std::string_view>>();

  terminal::flush();
  terminal::print("\x1b[{}A", y);
  y = 0;

  terminal::print("\r");

  std::size_t max_x = 0;
  for (std::string_view view_line : lines) {
    max_x = std::max(view_line.length(), max_x);

    if (x != 0) {
      terminal::print("\x1b[{}C", x);
    }

    terminal::print("{}\n", view_line);
  }
  terminal::flush();

  x += max_x;
  y += count_lines;
}
} // namespace text