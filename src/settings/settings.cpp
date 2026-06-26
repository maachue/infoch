#include "settings.hpp"

#include <fmt/format.h>

#include "config/conf.hpp"

namespace settings {
Settings set_from_conf(config::Image const &img, config::Text const &text) {

  if (img.cell_w > UINT16_MAX || img.cell_w < 0) {
    throw std::runtime_error(
        fmt::format("(validate_config) overflow image width: {}", img.cell_w));
  }

  if (img.cell_h > UINT16_MAX || img.cell_h < 0) {
    throw std::runtime_error(
        fmt::format("(validate_config) overflow image height: {}", img.cell_h));
  }

  if (img.padding_w > UINT16_MAX || img.padding_w < 0) {
    throw std::runtime_error(fmt::format(
        "(validate_config) overflow image padding left or right: {}",
        img.padding_w));
  }

  if (img.padding_h > UINT16_MAX || img.padding_h < 0) {
    throw std::runtime_error(fmt::format(
        "(validate_config) overflow image padding top: {}", img.padding_h));
  }

  if (text.padding_top > UINT16_MAX || text.padding_top < 0) {
    throw std::runtime_error(fmt::format(
        "(validate_config) overflow text padding top: {}", text.padding_top));
  }

  Settings result{};

  result.image.path = img.path;
  result.image.cell_width = static_cast<std::uint16_t>(img.cell_w);
  result.image.cell_height = static_cast<std::uint16_t>(img.cell_h);
  result.image.padding_width = static_cast<std::uint16_t>(img.padding_w);
  result.image.padding_height = static_cast<std::uint16_t>(img.padding_h);
  result.text_padding_t = static_cast<std::uint16_t>(text.padding_top);
  result.text_str = text.str;

  return result;
}
} // namespace settings