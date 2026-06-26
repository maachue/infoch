#ifndef INFOCH_SETTINGS_SETTINGS_H
#define INFOCH_SETTINGS_SETTINGS_H

#include <string_view>

#include "settings/image.hpp"

namespace config {
struct Image;
struct Text;
} // namespace config

namespace settings {
struct Settings {
  Image image;
  std::uint16_t text_padding_t;
  std::string_view text_str;
};

settings::Settings set_from_conf(config::Image const &img,
                                 config::Text const &text);
} // namespace settings

#endif
