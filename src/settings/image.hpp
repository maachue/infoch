#ifndef INFOCH_SETTINGS_IMAGE_H
#define INFOCH_SETTINGS_IMAGE_H

#include <cstdint>
#include <filesystem>

#include "image/types.hpp"

namespace settings {
struct Image {
  std::filesystem::path path;
  image::ImageType type;
  bool image_not_keep_aspect;

  std::uint16_t cell_width;
  std::uint16_t cell_height;

  std::uint16_t padding_width;
  std::uint16_t padding_height;
};
} // namespace settings

#endif
