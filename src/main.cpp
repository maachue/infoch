#include <exception>

#include <fmt/base.h>

#include <Magick++/Functions.h>

extern "C" {
#include <lua.h>
}

#include "cli.hpp"
#include "image/image.hpp"
#include "settings/image.hpp"
#include "terminal/cbreak_mode.hpp"
#include "terminal/detection.hpp"
#include "terminal/io.hpp"
#include "terminal/term_size.hpp"
#include "terminal/tty.hpp"

int main(int argc, char **argv) {
  cli::Cli cli = cli::cli_parse(argc, argv);

  try {
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

    terminal::fetch_terminal_size();

    auto const term = terminal::get_terminal();
    terminal::println("TERM:\n{}\nEND", term);

    std::uint16_t x = 0;
    std::uint16_t y = 0;

    settings::Image image_set{.path = cli.image,
                              .type = cli.image_type,
                              .cell_width = cli.image_width,
                              .cell_height = cli.image_height,
                              .padding_width = cli.image_padding_left,
                              .padding_height = cli.image_padding_top};

    std::exception_ptr ptr = nullptr;
    image::print_image(image_set, x, y, ptr);
    if (ptr) {
      std::rethrow_exception(ptr);
    }

    terminal::print("!!!!"); // check cursor postion after print (debug)
    terminal::flush();

    terminal::println("\n\n\n{}x{}", x, y);
  } catch (std::exception const &err) {
    fmt::println(stderr, "\x1b[31;1merror:\x1b[0m {}.", err.what());
    return 1;
  }
}
