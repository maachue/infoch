#ifndef INFOCH_CLI_UTILS_H
#define INFOCH_CLI_UTILS_H

#include <span>
#include <string_view>

namespace cli {
struct Cli;
} // namespace cli

namespace cli::internal {
using CallbackFnPtr = void (*)(Cli &cli, std::string_view str);

struct CliOpt {
  std::string_view long_opt;
  CallbackFnPtr callback;
  std::string_view short_opt;
  bool need_value;

  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  constexpr CliOpt(std::string_view longopt, std::string_view shrtopt, bool val,
                   CallbackFnPtr callback)
      // NOLINTEND(bugprone-easily-swappable-parameters)
      : long_opt(longopt), short_opt(shrtopt), need_value(val),
        callback(callback) {}

  [[nodiscard]] bool match_short(char ch) const;
  [[nodiscard]] bool match_long(std::string_view str) const;

  size_t solve_short(Cli &cli, std::span<std::string_view> args, size_t &idx,
                     std::string_view curr) const;

  void solve_long(Cli &cli, std::span<std::string_view> args, size_t &idx,
                  std::string_view curr) const;
};
} // namespace cli::internal

#endif
