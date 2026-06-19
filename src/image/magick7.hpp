#ifndef INFOCH_IMAGE_MAGICK7_H
#define INFOCH_IMAGE_MAGICK7_H

#include <Magick++/Blob.h>

namespace image::internal {
Magick::Blob magick_image(const char *path, const char *type,
                          bool keep_aspect_radio, size_t &width,
                          size_t &height);
} // namespace image::internal

#endif
