#ifndef INFOCH_IMAGE_TYPES_H
#define INFOCH_IMAGE_TYPES_H

#include <cstddef>

namespace image {
enum class ImageType { Auto, Disable, Kitty, Iterm, Sixel, Chafa, KittyPath };

struct DetailedImageSize {
  size_t cell_width;
  size_t cell_height;
  size_t pixel_width;
  size_t pixel_height;
};
} // namespace image

#endif
