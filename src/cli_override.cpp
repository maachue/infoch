#include "cli_override.hpp"

#include "cli.hpp"
#include "settings/settings.hpp"

namespace cli {
void cli_override(settings::Settings &set, Cli const &cli) {
  if (!cli.image.empty()) {
    set.image.path = cli.image;
  }

  if (cli.image_width) {
    set.image.cell_width = cli.image_width;
  }

  if (cli.image_height) {
    set.image.cell_height = cli.image_height;
  }

  if (cli.image_padding_left) {
    set.image.padding_width = cli.image_padding_left;
  }

  if (cli.image_padding_top) {
    set.image.padding_height = cli.image_padding_top;
  }
}
} // namespace cli