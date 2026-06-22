#ifndef INFOCH_TERMINAL_DECTECTION_H
#define INFOCH_TERMINAL_DECTECTION_H

#include <string_view>

#include <fmt/base.h>

namespace terminal {
struct Terminal {
  std::string_view name;
  bool is_tmux;
  bool is_screen;
  bool is_ssh;
  bool support_sixel;
  bool support_iterm;
  bool support_kitty;
};

Terminal const &get_terminal(bool force_screen = false, bool force_tmux = false,
                             bool force_ssh = false);
} // namespace terminal

template <> struct fmt::formatter<terminal::Terminal> {
  constexpr auto parse(fmt::format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const terminal::Terminal &c, FormatContext &ctx) const {
    return fmt::format_to(ctx.out(),
                          "name: {}\ntmux: {}\nscreen: {}\nssh: {}\nsixel | "
                          "kitty | iterm: {} {} {}\n",
                          c.name, c.is_tmux, c.is_screen, c.is_ssh,
                          c.support_sixel, c.support_kitty, c.support_iterm);
  }
};

#endif
