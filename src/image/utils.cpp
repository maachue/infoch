#include "utils.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include <Magick++/Image.h>

#include <fmt/core.h>

#include "terminal/term_size.hpp"

namespace image::internal {
void get_size_from_cell_size(DetailedImageSize &in, size_t image_w,
                             size_t image_h) {
  terminal::TermSize termsize = terminal::termsize;
  if (termsize.is_zero()) {
    terminal::fetch_terminal_size();

    termsize = terminal::termsize;
    if (termsize.is_zero()) {
      throw std::runtime_error(
          "(get_size_from_cell_size) terminal size is zero!");
    }
  }

  auto ratio = static_cast<double>(image_h) / image_w; // NOLINT
  if (std::isnan(ratio)) {
    throw std::runtime_error("(get_size_from_cell_size) NaN when calculated "
                             "aspect radio of input image");
  }

  size_t &req_cell_w = in.cell_width;
  size_t &req_cell_h = in.cell_height;

  if (req_cell_w != 0 && req_cell_h != 0) {
    in.pixel_width = std::ceil(
        (static_cast<double>(termsize.pixel_width) / termsize.cell_width) *
        req_cell_w); // NOLINT

    in.pixel_height = std::ceil(
        (static_cast<double>(termsize.pixel_height) / termsize.cell_height) *
        req_cell_h); // NOLINT
  } else if (req_cell_w != 0) {
    in.pixel_width = std::ceil(
        (static_cast<double>(termsize.pixel_width) / termsize.cell_width) *
        req_cell_w);                                               // NOLINT
    in.pixel_height = static_cast<double>(in.pixel_width) * ratio; // NOLINT
    in.cell_height =
        std::ceil(in.pixel_height /                            // NOLINT
                  (static_cast<double>(termsize.pixel_height / // NOLINT
                                       termsize.cell_height)));
  } else if (req_cell_h != 0) {
    in.pixel_height = std::ceil(
        (static_cast<double>(termsize.pixel_height) / termsize.cell_height) *
        req_cell_h);                                               // NOLINT
    in.pixel_width = static_cast<double>(in.pixel_height) / ratio; // NOLINT
    in.cell_width =
        std::ceil(in.pixel_width /                            // NOLINT
                  (static_cast<double>(termsize.pixel_width / // NOLINT
                                       termsize.cell_width)));
  } else {
    in.pixel_width = image_w;
    in.pixel_height = image_h;
    in.cell_width =
        std::ceil(in.pixel_width /                            // NOLINT
                  (static_cast<double>(termsize.pixel_width / // NOLINT
                                       termsize.cell_width)));
    in.cell_height =
        std::ceil(in.cell_height /                             // NOLINT
                  (static_cast<double>(termsize.pixel_height / // NOLINT
                                       termsize.cell_height)));
  }
}
} // namespace image::internal
