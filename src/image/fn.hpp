#ifndef INFOCH_IMAGE_FN_H
#define INFOCH_IMAGE_FN_H

#include <cstdint>
#include <filesystem>
#include <tuple>

namespace image::internal {
std::tuple<size_t, size_t>
get_size_from_cell_size(std::filesystem::path const &path, std::uint16_t &width,
                        std::uint16_t &height);

void unicode_print_image(std::filesystem::path const &path, size_t, size_t);
void kitty_print_image(std::filesystem::path const &path, size_t width,
                       size_t height, size_t cell_width, size_t cell_height);
// NOTE: image size for kitty_path_print_image must be cell size, not pixel size
void kitty_path_print_image(std::filesystem::path const &path,
                            size_t pixel_width, size_t pixel_height);
void iterm_print_image(std::filesystem::path const &path, size_t width,
                       size_t height);
// NOTE: image size for chafa_print_image must be cell size, not pixel size
void chafa_print_image(std::filesystem::path const &path, size_t width,
                       size_t height);
void sixel_print_image(std::filesystem::path const &path, size_t width,
                       size_t height);
} // namespace image::internal

#endif
