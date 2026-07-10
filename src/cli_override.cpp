#include "cli_override.hpp"

#include "cli.hpp"
#include "settings/settings.hpp"

namespace cli {
void cli_override(settings::Settings &set, Cli const &cli) {
  if (!cli.image.empty()) {
    set.image.path = cli.image;
  }

  set.image.type = cli.image_type.value_or(set.image.type);

  set.image.image_not_keep_aspect = cli.image_not_keep_aspect;

  set.image.cell_width = cli.image_width.value_or(set.image.cell_width);
  set.image.cell_height = cli.image_height.value_or(set.image.cell_height);
  set.image.padding_width =
      cli.image_padding_left.value_or(set.image.padding_width);
  set.image.padding_height =
      cli.image_padding_top.value_or(set.image.padding_height);
}
} // namespace cli