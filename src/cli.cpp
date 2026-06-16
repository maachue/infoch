#include "cli.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <expected>
#include <filesystem>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>

#include <fmt/base.h>
#include <fmt/format.h>

#include "cli_utils.hpp"
#include "image/types.hpp"

namespace cli {
constexpr std::string_view kBoldUnderline = "\x1b[1;4m";
constexpr std::string_view kBold = "\x1b[1m";
constexpr std::string_view kReset = "\x1b[0m";
constexpr std::string_view kItalic = "\x1b[3m";
constexpr std::string_view kRedBold = "\x1b[1;31m";

using namespace fmt::literals;
using namespace internal;

void usage() {
  fmt::print("{1}Usage:{0} {2}infoch{0} [options]\n\n", kReset, kBoldUnderline,
             kBold);
}

void help_msg() {
  // clang-format off
  fmt::print("{2}Options:{0}\n"
             "  {1}-h, --help{0}                                 Show this help message\n"
             "  {1}-c, --config{0} {3}<config>{0}                      Specify the config file to load\n"
             "  {1}-i, --image{0} {3}<logo>{0}                         Set the image source. \"none\" to disable\n"
             "  {1}-T,  --image-type{0} {3}<enum>{0}                    Set type of iamge\n"
             "  {1}    --image-width{0} {3}<num>{0}                    Set the width of the image in cells\n"
             "  {1}    --image-height{0} {3}<num>{0}                   Set the height of the image in cells\n",

             kReset, kBold, kBoldUnderline, kItalic);
  // clang-format on
}

void version_msg() {
  fmt::print("infoch {}\n"
             "Lua version: {}\n",
             "0.1.0", "N/A");
}

constexpr std::array kCliArgs = {
    CliOpt("config", "c", true,
           [](Cli &cli, std::string_view str) {
             cli.config = std::filesystem::path(str);
           }),
    CliOpt("image", "i", true,
           [](Cli &cli, std::string_view str) {
             cli.image = std::filesystem::path(str);
           }),
    CliOpt("image-type", "T", true,
           [](Cli &cli, std::string_view str) {
             if (str == "none") {
               cli.image_type = image::ImageType::Disable;
               return;
             }

             if (str == "kitty" || str == "Kitty") {
               cli.image_type = image::ImageType::Kitty;
               return;
             }

             // if (str == "kitty-icat" || str == "KittyIcat" ||
             //     str == "kitty_icat") {
             //   cli.image_type = image::ImageType::KittyIcat;
             //   return;
             // }

             if (str == "sixel" || str == "Sixel") {
               cli.image_type = image::ImageType::Sixel;
               return;
             }

             if (str == "iterm" || str == "Iterm") {
               cli.image_type = image::ImageType::Iterm;
               return;
             }

             throw std::runtime_error(
                 fmt::format("unknown \"--image-type\" value: {}", str));
           }),
    CliOpt("image-width", {}, true,
           [](Cli &cli, std::string_view str) {
             auto [ptr, err] = std::from_chars(
                 str.data(), str.data() + str.length(), cli.image_width);
             if (err != std::errc{}) {
               throw std::system_error(
                   std::make_error_code(err),
                   fmt::format(R"(invalid "--image-width" value "{}")", str));
             }
           }),
    CliOpt("image-height", {}, true, [](Cli &cli, std::string_view str) {
      auto [ptr, err] = std::from_chars(str.data(), str.data() + str.length(),
                                        cli.image_height);
      if (err != std::errc{}) {
        throw std::system_error(
            std::make_error_code(err),
            fmt::format(R"(invalid "--image-height" value "{}")", str));
      }
    })};

// NOLINTBEGIN(readability-function-cognitive-complexity)
std::expected<Cli, int> cli_parse(int argc, char **argv) noexcept {
  // NOLINTEND(readability-function-cognitive-complexity)
  try {
    Cli cli{};
    cli.image_type = image::ImageType::Kitty;
    cli.image_width = 20;
    cli.image_height = 10;

    std::string_view arg;
    std::vector<std::string_view> vec =
        std::ranges::views::counted(std::next(argv), argc - 1) |
        std::ranges::to<std::vector<std::string_view>>();
    for (size_t i = 0; i < vec.size(); ++i) {
      arg = vec[i];

      if (arg == "--help" || arg == "-h") {
        usage();
        help_msg();
        return std::unexpected{0};
      }

      if (arg == "-v" || arg == "--version") {
        version_msg();
        return std::unexpected{0};
      }

      if (arg.length() > 2 && arg.starts_with("--")) {
        arg.remove_prefix(2); // remove `--`
        const auto *ptr = std::ranges::find_if(
            kCliArgs, [&](auto const &opt) { return opt.match_long(arg); });

        if (ptr != kCliArgs.end()) {
          ptr->solve_long(cli, vec, i, arg);
          continue;
        }
      } else if (arg.length() > 1 && arg[0] == '-') {
        arg.remove_prefix(1); // remove `-`

        for (size_t j = 0; j < arg.size(); j++) {
          char t = arg[j];

          if (t == '=') {
            throw std::runtime_error(fmt::format(
                R"(unknown argument: "{}" (unexpected '=' in "-{}"))", vec[i],
                arg));
          }

          const auto *ptr = std::ranges::find_if(
              kCliArgs, [&](auto const &opt) { return opt.match_short(t); });

          if (ptr != kCliArgs.end()) {
            auto a = ptr->solve_short(cli, vec, i, arg.substr(j));
            j += a - 1;
            continue;
          }

          throw std::runtime_error(fmt::format(
              R"(unknown argument: "{}" (unknown "-{}" option))", vec[i], t));
        }

        continue;
      }

      throw std::runtime_error(fmt::format("unknown argument: \"{}\"", vec[i]));
    }

    return cli;
  } catch (std::exception const &err) {
    fmt::println("\x1b[31;1merror\x1b[0m: {}.", err.what());
    return std::unexpected{1};
  }
}
} // namespace cli
