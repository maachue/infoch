#ifndef INFOCH_CLI_H
#define INFOCH_CLI_H

#include <filesystem>
#include <optional>

#include "image/types.hpp"

namespace cli {
struct Cli {
  std::filesystem::path config;
  bool no_config;

  // debug
  bool no_redirect;
  bool no_check_stdout;

  // image
  std::filesystem::path image;
  std::optional<image::ImageType> image_type;
  bool image_not_keep_aspect;

  std::optional<std::uint16_t> image_width;
  std::optional<std::uint16_t> image_height;

  std::optional<std::uint16_t> image_padding_left;
  std::optional<std::uint16_t> image_padding_top;
};

Cli cli_parse(int argc, char **argv) noexcept;
} // namespace cli

#endif
