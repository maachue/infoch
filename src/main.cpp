#include <Magick++/Functions.h>
#include <exception>

#include <fmt/base.h>
#include <stdexcept>
#include <system_error>

#include <Magick++.h>

#include "cli.hpp"
#include "image/image.hpp"
#include "settings/image.hpp"
#include "terminal/cbreak_mode.hpp"
#include "terminal/io.hpp"
#include "terminal/term_size.hpp"
#include "terminal/tty.hpp"

int main(int argc, char **argv) {
  cli::Cli cli{};
  {
    auto res = cli::cli_parse(argc, argv);

    if (!res) {
      return res.error();
    }

    cli = res.value();
  }

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
        terminal::print("\x1b[?7l\x1b[>1u");
        terminal::flush();
      }
      ~OptinalConfigureTerm() {
        terminal::print("\x1b[?7h\x1b[<1u");
      } // this will flush when BuffLifetime::~BuffLifetime() call
    } opt_config_do_not_touch{};

    std::error_code err{};
    auto d = terminal::fetch_terminal_size(err);

    if (d != 0) {
      if (err) {
        throw std::system_error(err, "failed to fetch terminal size");
      }

      throw std::runtime_error("failed to fetch terminal size by unkown error");
    }

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

    terminal::print("!!!!"); // check cursor postion after print (debug)
    terminal::flush();

    terminal::println("\n\n\n{}x{}", x, y);
    if (ptr) {
      std::rethrow_exception(ptr);
    }
  } catch (std::exception const &err) {
    fmt::println(stderr, "\x1b[31;1merror:\x1b[0m {}.", err.what());
  }
}
