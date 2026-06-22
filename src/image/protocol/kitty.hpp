#ifndef INFOCH_IMAGE_PROTOCOL_KITTY_H
#define INFOCH_IMAGE_PROTOCOL_KITTY_H

#include <filesystem>
#include <span>

namespace image {
struct DetailedImageSize;

namespace internal {
/// @note kitty path doesn't need preload image, just need path and size
void kitty_path_print_image(std::filesystem::path const &path,
                            DetailedImageSize &size);

/// @param bytes MUST BASE64 ENCODED, RGBA (32-bit) (NOT PNG OR JPG, OR RGB
/// (24-bit))
void kitty_print_image(std::span<const char> bytes, DetailedImageSize &size);

/// @param bytes MUST BASE64 ENCODED, RGBA (32-bit) (NOT PNG OR JPG, OR RGB
/// (24-bit))
void tmux_kitty_print_image(std::span<const char> bytes,
                            DetailedImageSize &size);
} // namespace internal
} // namespace image

#endif