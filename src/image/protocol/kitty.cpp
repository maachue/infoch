#include "kitty.hpp"

#include <filesystem>

#include "base64.hpp"
#include "image/types.hpp"
#include "terminal/detection.hpp"
#include "terminal/io.hpp"
#include "terminal/passthrough.hpp"

using terminal::kTmuxPassthroughBegin;
using terminal::kTmuxPassthroughEnd;

using namespace fmt::literals;

namespace image::internal {
void kitty_path_print_image(std::filesystem::path const &path,
                            DetailedImageSize &size) {
  std::string p{};
  {
    auto str = path.u8string();
    p = base64_encode(
        {reinterpret_cast<const char *>(str.data()), str.length()});
  }

  const auto &t = terminal::get_terminal();
  if (t.is_tmux) {
    terminal::print("{}", kTmuxPassthroughBegin);
  }

  if (size.cell_width != 0 && size.cell_height == 0) {
    terminal::print("\x1b_Gf=100,a=T,t=f,c={};{}\x1b\\", size.cell_width, p);
  } else if (size.cell_height != 0 && size.cell_width == 0) {
    terminal::print("\x1b_Gf=100,a=T,t=f,r={};{}\x1b\\", size.cell_height, p);
  } else {
    terminal::print("\x1b_Gf=100,a=T,t=f,r={},c={};{}\x1b\\", size.cell_height,
                    size.cell_width, p);
  }

  if (t.is_tmux) {
    terminal::print("{}", kTmuxPassthroughEnd);
  }

  terminal::flush();
}

constexpr size_t kKittyMaxBlob = 4096;

void kitty_print_image(std::span<const char> bytes, DetailedImageSize &size) {
  size_t remaining_length = bytes.size();
  const auto *pos = bytes.data();

  terminal::print("\x1b_Gf=32,a=T,s={},v={},q=2,m=1\x1b\\", size.pixel_width,
                  size.pixel_height);

  size_t chunk = 4096;
  while (remaining_length > 0) {
    chunk = std::min(remaining_length, kKittyMaxBlob);

    terminal::print("\x1b_Gm=1;{:.{}}\x1b\\", pos, chunk);
    terminal::flush();

    pos += chunk;
    remaining_length -= chunk;
  }

  terminal::print("\x1b_Gm=0\x1b\\");

  terminal::flush();
}

void tmux_kitty_print_image(std::span<const char> bytes,
                            DetailedImageSize &size) {
  return;

  size_t remaining_length = bytes.size();
  const auto *pos = bytes.data();

  terminal::print("{TMUX_BEGIN}\x1b\x1b_Gf=32,a=T,U=1,s={},v={}c={},r={},q=2,m="
                  "1\x1b\x1b\\{TMUX_END}",
                  "TMUX_BEGIN"_a = kTmuxPassthroughBegin,
                  "TMUX_END"_a = kTmuxPassthroughEnd, size.pixel_width,
                  size.pixel_height, size.cell_width, size.cell_height);

  size_t chunk = 4096;
  while (remaining_length > 0) {
    chunk = std::min(remaining_length, kKittyMaxBlob);

    terminal::print("{TMUX_BEGIN}\x1b\x1b_Gm=1;{}\x1b\x1b\\{TMUX_END}",
                    "TMUX_BEGIN"_a = kTmuxPassthroughBegin,
                    "TMUX_END"_a = kTmuxPassthroughEnd,
                    std::string_view(pos, chunk));
    terminal::flush();

    pos += chunk;
    remaining_length -= chunk;
  }

  terminal::print("{TMUX_BEGIN}\x1b\x1b_Gm=0\x1b\x1b\\{TMUX_END}",
                  "TMUX_BEGIN"_a = kTmuxPassthroughBegin,
                  "TMUX_END"_a = kTmuxPassthroughEnd);

  terminal::flush();
}
} // namespace image::internal