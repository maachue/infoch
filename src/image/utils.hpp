#ifndef INFOCH_IMAGE_FN_H
#define INFOCH_IMAGE_FN_H

#include "image/types.hpp"

namespace image::internal {
void get_size_from_cell_size(DetailedImageSize &in, size_t image_w,
                             size_t image_h);
} // namespace image::internal

#endif
