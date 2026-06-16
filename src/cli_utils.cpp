#include "cli_utils.hpp"

#include <cassert>
#include <stdexcept>
#include <string_view>

#include <fmt/base.h>
#include <fmt/format.h>

namespace cli::internal {
[[nodiscard]] bool CliOpt::match_short(char ch) const {
  if (short_opt.empty()) {
    return false;
  }

  return short_opt[0] == ch;
}

[[nodiscard]] bool CliOpt::match_long(std::string_view str) const {
  if (need_value) {
    if (str.length() > long_opt.length()) {
      return str.starts_with(long_opt) && str[long_opt.length()] == '=';
    }
    return str.starts_with(long_opt);
  }

  return str == long_opt;
}

size_t CliOpt::solve_short(Cli &cli, std::span<std::string_view> args,
                           size_t &idx, std::string_view curr) const {
  if (need_value) {
    if (curr.length() == 1) {
      idx++;
      if (idx >= args.size()) {
        throw std::runtime_error(
            fmt::format("\"-{}\" required an argument", short_opt));
      }

      callback(cli, args[idx]);
      return curr.length();
    }

    {
      auto pos = curr.find('=');
      if (pos != std::string_view::npos) {
        // NOTE: fix edge case:
        // program -aVALUE=VALUE
        // The value of -a must be VALUE=VALUE, not VALUE
        auto key = curr.substr(0, pos);
        auto value = curr.substr(pos + 1);
        if (key.length() > 1) {
          value = curr.substr(1);
        }

        callback(cli, value);
        return curr.length();
      }
    }

    std::string_view val = curr.substr(1);
    callback(cli, val);
    return curr.length();
  }

  callback(cli, {});
  return 1;
}

void CliOpt::solve_long(Cli &cli, std::span<std::string_view> args, size_t &idx,
                        std::string_view curr) const {
  if (need_value) {
    if (curr.length() == long_opt.length()) {
      idx++;
      if (idx >= args.size()) {
        throw std::runtime_error(
            fmt::format("\"--{}\" required an argument", long_opt));
      }

      callback(cli, args[idx]);
      return;
    }

    if (curr[long_opt.length()] == '=') {
      auto value = curr.substr(long_opt.length() + 1);
      callback(cli, value);
      return;
    }

    // auto pos = curr.find('=');
    // if (pos != std::string_view::npos) {
    //   auto value = curr.substr(pos + 1);
    //   callback(cli, value);
    //   return;
    // }

    throw std::runtime_error(
        fmt::format("\"--{}\" required an argument", long_opt));
  }

  callback(cli, {});
}
} // namespace cli::internal
