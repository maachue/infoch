#include <exception>

#include <fmt/base.h>

#include "cli.hpp"
#include "terminal/cbreak_mode.hpp"
#include "terminal/io.hpp"
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
      terminal::init_buff(true);
    }

    struct Obj {
      Obj() = default;
      ~Obj() {
        terminal::deinit_cbreak_mode();
        terminal::close_devtty();
        terminal::deinit_buff();
      }
    } do_not_touch{};

    terminal::print("Config: {}\n"
                      "Image: {}\n"
                      "Image type: {}\n"
                      "Image width X height: {}x{}\n",
                      cli.config.string(), cli.image.string(), cli.image_type,
                      cli.image_width, cli.image_height);

    fmt::println("Hello, World!");
  } catch (std::exception const &err) {
    fmt::println(stderr, "\x1b[31;1merror:\x1b[0m {}.", err.what());
  }
}
