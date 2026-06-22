#include "fn.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <tuple>

#include <Magick++/Image.h>

#include <fmt/core.h>

#include "base64.hpp"
#include "image/magick7.hpp"
#include "terminal/detection.hpp"
#include "terminal/io.hpp"
#include "terminal/passthrough.hpp"
#include "terminal/term_size.hpp"

namespace image::internal {
std::tuple<size_t, size_t>
get_size_from_cell_size(const std::filesystem::path &path, std::uint16_t &width,
                        std::uint16_t &height) {
  terminal::TermSize termsize = terminal::termsize;
  assert(!termsize.is_zero());

  size_t pwidth = 0;
  size_t pheight = 0;

  size_t iwidth = 0;
  size_t iheight = 0;

  {
    Magick::Image image;
    {
      auto stt = path.u8string();
      std::string path_s(stt.begin(), stt.end());
      image.read(path_s);
    }

    iwidth = image.columns();
    iheight = image.rows();
  }

  auto ratio = static_cast<double>(iheight) / iwidth; // NOLINT
  if (std::isnan(ratio)) {
    throw std::runtime_error("(get_size_from_cell_size) NaN when calculated "
                             "aspect radio of input image");
  }

  if (width != 0 && height != 0) {
    pwidth = std::ceil(
        (static_cast<double>(termsize.pixel_width) / termsize.cell_width) *
        width); // NOLINT

    pheight = std::ceil(
        (static_cast<double>(termsize.pixel_height) / termsize.cell_height) *
        height); // NOLINT
  } else if (width != 0) {
    pwidth = std::ceil(
        (static_cast<double>(termsize.pixel_width) / termsize.cell_width) *
        width);                                                     // NOLINT
    pheight = static_cast<double>(pwidth) * ratio;                  // NOLINT
    height = std::ceil(pheight /                                    // NOLINT
                       (static_cast<double>(termsize.pixel_height / // NOLINT
                                            termsize.cell_height)));
  } else if (height != 0) {
    pheight = std::ceil(
        (static_cast<double>(termsize.pixel_height) / termsize.cell_height) *
        height);                                                  // NOLINT
    pwidth = static_cast<double>(pheight) * ratio;                // NOLINT
    width = std::ceil(pwidth /                                    // NOLINT
                      (static_cast<double>(termsize.pixel_width / // NOLINT
                                           termsize.cell_width)));
  } else {
    pwidth = iwidth;
    pheight = iheight;
    width = std::ceil(pwidth /                                    // NOLINT
                      (static_cast<double>(termsize.pixel_width / // NOLINT
                                           termsize.cell_width)));
    height = std::ceil(pheight /                                    // NOLINT
                       (static_cast<double>(termsize.pixel_height / // NOLINT
                                            termsize.cell_height)));
  }

  return {pwidth, pheight};
}

void kitty_print_image(const std::filesystem::path &path, size_t pixel_width,
                       size_t pixel_height, size_t cell_width,
                       size_t cell_height) {

  constexpr size_t kKittyMaxBlob = 4096;

  auto blob = image::internal::magick_image(path.u8string(), "RGBA", true,
                                            pixel_width, pixel_height);
  auto str = blob.base64();

  size_t remaining_length = str.length();
  const auto *pos = str.data();

  const auto &term = terminal::get_terminal();

  if (term.is_tmux) {
    terminal::print(
        "{}\x1b\x1b_Gf=32,a=T,i=31,s={},v={},c={},r={},U=1,q=2,m=1\x1b\x1b\\{}",
        terminal::kTmuxPassthroughBegin, pixel_width, pixel_height, cell_width,
        cell_height, terminal::kTmuxPassthroughEnd);
  } else {
    terminal::print("\x1b_Gf=32,a=T,s={},v={},q=2,m=1\x1b\\", pixel_width,
                    pixel_height);
  }

  size_t chunk = 4096;
  if (term.is_tmux) {
    while (remaining_length > 0) {
      chunk = std::min(remaining_length, kKittyMaxBlob);

      terminal::print("{}\x1b\x1b_Gm=1;{:.{}}\x1b\x1b\\{}",
                      terminal::kTmuxPassthroughBegin, pos, chunk,
                      terminal::kTmuxPassthroughEnd);
      terminal::flush();

      pos += chunk;
      remaining_length -= chunk;
    }
  } else {
    while (remaining_length > 0) {
      chunk = std::min(remaining_length, kKittyMaxBlob);

      terminal::print("\x1b_Gm=1;{:.{}}\x1b\\", pos, chunk);
      terminal::flush();

      pos += chunk;
      remaining_length -= chunk;
    }
  }

  if (term.is_tmux) {
    terminal::print("{}\x1b\x1b_Gm=0\x1b\x1b\\{}\x1b\\",
                    terminal::kTmuxPassthroughBegin,
                    terminal::kTmuxPassthroughEnd);
  } else {
    terminal::print("\x1b_Gm=0\x1b\\");
  }

  terminal::flush();
}

void kitty_path_print_image(std::filesystem::path const &path, size_t width,
                            size_t height) {
  // f=100: PNG
  // f=32: RGBA
  // f=24: RGB

  auto str = path.string();
  str = base64_encode({str.data(), str.length()});
  if (width != 0 && height == 0) {
    terminal::print("\x1b_Gf=100,a=T,t=f,c={};{}\x1b\\", width, str);
  } else if (height != 0 && width == 0) {
    terminal::print("\x1b_Gf=100,a=T,t=f,r={};{}\x1b\\", height, str);
  } else {
    terminal::print("\x1b_Gf=100,a=T,t=f,r={},c={};{}\x1b\\", height, width,
                    str);
  }

  terminal::flush();
}

void iterm_print_image(std::filesystem::path const &path, size_t width,
                       size_t height) {
  auto blob = image::internal::magick_image(path.u8string(), "PNG", true, width,
                                            height);
  auto str = blob.base64();

  terminal::print("\x1b]1337;File=inline=1:{}\a", str);
}

void sixel_print_image(std::filesystem::path const &path, size_t width,
                       size_t height) {
  auto blob = image::internal::magick_image(path.u8string(), "SIXEL", true,
                                            width, height);
  terminal::print("{}",
                  std::string_view(reinterpret_cast<const char *>(blob.data()),
                                   blob.length()));
}
} // namespace image::internal
