#ifndef INFOCH_SETTINGS_SETTINGS_H
#define INFOCH_SETTINGS_SETTINGS_H

#include <string>

#include "settings/image.hpp"

namespace config {
struct Image;
struct Text;
} // namespace config

namespace settings {
struct Settings {
  Image image;
  std::uint16_t text_padding_t;
  std::string text_str;
};

void run_config(std::filesystem::path const &path, Settings &set);
} // namespace settings

#endif
