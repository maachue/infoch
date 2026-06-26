#ifndef INFOCH_CONFIG_CONF_H
#define INFOCH_CONFIG_CONF_H

#include <filesystem>

namespace config {
struct Text {
  double padding_top = 0;
  std::string str;
};

struct Image {
  std::filesystem::path path;
  double cell_w;
  double cell_h;
  double padding_w;
  double padding_h;
};

extern constinit Image g_image;
extern constinit bool g_image_init;
extern constinit Text g_text;

void run_config(std::filesystem::path const &config);
} // namespace config

#endif