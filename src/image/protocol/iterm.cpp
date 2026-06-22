#include "iterm.hpp"

#include "image/types.hpp"
#include "terminal/detection.hpp"
#include "terminal/io.hpp"
#include "terminal/passthrough.hpp"

namespace image::internal {
void iterm_print_image(std::span<const char> bytes, DetailedImageSize &size) {
  auto const &term = terminal::get_terminal();

  if (term.is_tmux) {
    terminal::print("{}", terminal::kTmuxPassthroughBegin);
  }

  terminal::print("\x1b]1337;File=inline=1;width={};height={}:{}\a",
                  size.cell_width, size.cell_height,
                  std::string_view(bytes.data(), bytes.size()));

  if (term.is_tmux) {
    terminal::print("{}", terminal::kTmuxPassthroughEnd);
  }
}
} // namespace image::internal