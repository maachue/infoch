#ifndef INFOCH_IMAGE_TYPES_H
#define INFOCH_IMAGE_TYPES_H

#include <array>
#include <cstddef>
#include <string_view>

namespace image {
enum class ImageType { Auto, Disable, Kitty, Iterm, Sixel, Chafa, KittyPath };

constexpr std::array<std::string_view, 7> kImagetypeNameStrArr = {
    "auto", "disable", "kitty", "iterm", "sixel", "chafa", "kittypath"};

struct DetailedImageSize {
  size_t cell_width;
  size_t cell_height;
  size_t pixel_width;
  size_t pixel_height;
};
} // namespace image

#endif
