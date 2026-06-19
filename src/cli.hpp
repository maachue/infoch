#ifndef INFOCH_CLI_H
#define INFOCH_CLI_H

#include <expected>
#include <filesystem>

#include "image/types.hpp"

namespace cli {
struct Cli {
  std::filesystem::path config;

  // debug
  bool no_redirect;
  bool no_check_stdout;

  // image
  std::filesystem::path image;
  image::ImageType image_type;

  std::uint16_t image_width;
  std::uint16_t image_height;

  std::uint16_t image_padding_left;
  std::uint16_t image_padding_top;
};

std::expected<Cli, int> cli_parse(int argc, char **argv) noexcept;
} // namespace cli

#endif
