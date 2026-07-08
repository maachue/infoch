#include <exception>

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
#include "terminal/term_size.hpp"
#include "terminal/tty.hpp"

void nested_exception_print(std::exception const &err,
                            std::string_view context = "") {
  if (context.empty()) {
    fmt::print(stderr, "\x1b[31;1merror:\x1b[0m: {}", err.what());
  } else {
    fmt::print(stderr, "{}{}", context, err.what());
  }

  try {
    std::rethrow_if_nested(err);
  } catch (std::exception const &nested_err) {
    nested_exception_print(nested_err, ": ");
  }
}

int main(int argc, char **argv) {
  cli::Cli cli = cli::cli_parse(argc, argv);

  try {
    settings::Settings set{};

    settings::run_config(cli.config, set);

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
    try {
      terminal::fetch_terminal_size();
    } catch (std::exception const &err) {
      failed = true;
      nested_exception_print(err);
      fmt::println(stderr, ".");
    }

    if (!failed) {
      std::uint16_t x = 0;
      std::uint16_t y = 0;

      std::exception_ptr ptr = nullptr;
      image::print_image(set.image, x, y, ptr);
      if (ptr) {
        std::rethrow_exception(ptr);
      }

      terminal::print("!!!!"); // check cursor postion after print (debug)
      terminal::flush();

      terminal::println("\n\n\n{}x{}", x, y);
    }

    terminal::println("\n\n\n\n\n\n\n{}", set.text_str);
  } catch (std::exception const &err) {
    fmt::println(stderr, "\x1b[31;1merror:\x1b[0m {}.", err.what());
    return 1;
  }
}
