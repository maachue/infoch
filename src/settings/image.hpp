#ifndef INFOCH_SETTINGS_IMAGE_H
#define INFOCH_SETTINGS_IMAGE_H

#include <cstdint>
#include <filesystem>

#include "image/types.hpp"

namespace settings {
struct Image {
  std::filesystem::path path;
  image::ImageType type;

  std::uint16_t cell_width;
  std::uint16_t cell_height;

  std::uint16_t padding_cols;
  std::uint16_t padidng_rows;
};
} // namespace settings

#endif
