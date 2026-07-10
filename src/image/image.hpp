#ifndef INFOCH_IMAGE_IMAGE_H
#define INFOCH_IMAGE_IMAGE_H

#include <cstdint>

#include "settings/image.hpp"

namespace image {

void print_image(settings::Image &set, std::uint16_t &curr_x,
                 std::uint16_t &curr_y);
} // namespace image

#endif
