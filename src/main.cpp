#include <exception>

#include <fmt/base.h>
#include <stdexcept>
#include <system_error>

#include "cli.hpp"
#include "image/fn.hpp"
#include "image/image.hpp"
#include "settings/image.hpp"
#include "terminal/cbreak_mode.hpp"
#include "terminal/io.hpp"
#include "terminal/term_query.hpp"
#include "terminal/term_size.hpp"
#include "terminal/tty.hpp"

template <> struct fmt::formatter<image::ImageType> {
  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(const image::ImageType &p, format_context &ctx) const {
    using enum image::ImageType;
    std::string_view str{};

    switch (p) {
    case Kitty:
      str = "kitty";
      break;
    case Disable:
      str = "disable";
      break;
    case Auto:
      str = "auto";
      break;
    // case KittyIcat:
    //   str = "kitty-icat";
    //   break;
    case Sixel:
      str = "sixel";
      break;
    case Iterm:
      str = "iterm";
      break;
    default:
      str = "idk";
      break;
    }

    return fmt::format_to(ctx.out(), "{}", str);
  }
  // NOLINTEND(readability-convert-member-functions-to-static)
};

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
    {
      terminal::open_devtty();
      terminal::init_cbreak_mode();
      terminal::init_buff(cli.no_redirect);
      terminal::fetch_terminal_size();
    }

    struct Obj {
      Obj() {
        terminal::print("\x1b[?7l\x1b[2J\x1b[H");
        terminal::flush();
      }
      ~Obj() {
        terminal::deinit_cbreak_mode();
        terminal::close_devtty();
        terminal::deinit_buff();
      }
    } do_not_touch{};

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
