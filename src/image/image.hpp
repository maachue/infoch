#ifndef INFOCH_IMAGE_IMAGE_H
#define INFOCH_IMAGE_IMAGE_H

#include <cstdint>
#include <exception>

#include "settings/image.hpp"

namespace image {

void print_image(const settings::Image &set, std::uint16_t &curr_x,
                 std::uint16_t &curr_y, std::exception_ptr &err) noexcept;
} // namespace image

#endif
