#ifndef INFOCH_IMAGE_IMAGE_H
#define INFOCH_IMAGE_IMAGE_H

#include <exception>
#include <filesystem>
#include <system_error>

#include "image/types.hpp"
#include "settings/image.hpp"

namespace image {
namespace internal {
using PrintImagePtr = void (*)(std::error_code &err,
                               std::filesystem::path const &path, size_t width,
                               size_t height);

PrintImagePtr get_function_print_image_protocol(ImageType const type);
} // namespace internal

std::exception_ptr print_image(const settings::Image &set) noexcept;
} // namespace image

#endif
