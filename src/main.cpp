#include <clocale>
#include <exception>
#include <locale>

#include <fmt/base.h>

#include <Magick++/Functions.h>

extern "C" {
#include <lua.h>
}

#include "cli.hpp"
#include "cli_override.hpp"
#include "image/image.hpp"
#include "settings/settings.hpp"
#include "terminal/cbreak_mode.hpp"
#include "terminal/io.hpp"
#include "terminal/tty.hpp"
#include "text/text.hpp"

int main(int argc, char **argv) {
  if (std::setlocale(LC_CTYPE, "en_US.UTF-8") != nullptr) {
    std::locale::global(std::locale("en_US.UTF-8"));
  } else {
    if (std::setlocale(LC_CTYPE, "C.UTF-8") != nullptr) {
      std::locale::global(std::locale("C.UTF-8"));
    }
  }

  cli::Cli cli = cli::cli_parse(argc, argv);

  try {
    settings::Settings set{};

    if (!cli.no_config) {
      settings::run_config(cli.config, set);
    }

    cli::cli_override(set, cli);

    Magick::InitializeMagick(*argv);

    struct DevTTYLifetime {
      DevTTYLifetime() { terminal::open_devtty(); }
      ~DevTTYLifetime() { terminal::close_devtty(); }
    } dev_tty_life_time_do_not_touch{};

    struct CBreakModeLifetime {
      CBreakModeLifetime() { terminal::init_cbreak_mode(); }
      ~CBreakModeLifetime() { terminal::deinit_cbreak_mode(); }
    } cbreak_mode_life_time_do_not_touch{};

    struct BuffLifetime {
      BuffLifetime(cli::Cli &cli) { terminal::init_buff(cli.no_redirect); }
      ~BuffLifetime() { terminal::deinit_buff(); }
    } buff_lifetime_do_not_touch{cli};

    struct OptinalConfigureTerm {
      OptinalConfigureTerm() {
        terminal::print("\x1b[?7l");
        terminal::flush();
      }
      ~OptinalConfigureTerm() {
        terminal::print("\x1b[?7h");
      } // this will flush when BuffLifetime::~BuffLifetime() call
    } opt_config_do_not_touch{};

    bool failed = false;

    std::uint16_t x = 0;
    std::uint16_t y = 0;

    try {
      image::print_image(set.image, x, y);
    } catch (std::exception const &err) {
      fmt::println(stderr,
                   "\x1b[31;1merror:\x1b[0m Something went wrong when "
                   "printing image: {}",
                   err.what());
    }

    if (!cli.no_text) {
      text::print_text(set.text_str, x, y);
    }
  } catch (std::exception const &err) {
    fmt::println(stderr, "\x1b[31;1merror:\x1b[0m {}", err.what());
    return 1;
  }
}
