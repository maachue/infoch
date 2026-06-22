#ifndef INFOCH_IMAGE_PROTOCOL_ITERM_H
#define INFOCH_IMAGE_PROTOCOL_ITERM_H

#include <span>

namespace image {
struct DetailedImageSize;

namespace internal {
/// WARNING:
/// @param bytes MUST BASE64 ENCODED, TIFF, PNG OR ANY IMAGE TYPES SUPPORTED BY
/// ITERM2
/// @see https://iterm2.com/3.4/documentation-images.html
void iterm_print_image(std::span<const char> bytes, DetailedImageSize &size);
} // namespace internal
} // namespace image

#endif