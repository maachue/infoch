#include "magick7.hpp"

#include <cassert>

#include <Magick++/Blob.h>
#include <Magick++/Geometry.h>
#include <Magick++/Image.h>

#include <fmt/format.h>

using Magick::Blob;
using Magick::Geometry;
using Magick::Image;

namespace image::internal {
Blob magick_image(std::u8string &&path, const char *type,
                  bool keep_aspect_radio, size_t &width, size_t &height) {
  assert(path.empty() && type != nullptr &&
         "(image/magick7:magick_image) shit caller, are you dumb?");
  Image image;
  image.read(std::string(path.begin(), path.end()));
  if (keep_aspect_radio) {
    image.resize(Geometry(width, height));
  } else {
    image.resize(fmt::format("{}x{}!", width, height));
  }
  image.magick(type);

  width = image.columns();
  height = image.rows();

  Blob blob;
  image.write(&blob);
  return blob;
}
} // namespace image::internal
