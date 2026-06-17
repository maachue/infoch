#ifndef INFOCH_TERMINAL_IO_H
#define INFOCH_TERMINAL_IO_H

#include <fmt/base.h>
#include <fmt/format.h>

namespace terminal {
void init_buff(bool bypass);
void deinit_buff();
void vprint(fmt::string_view fmt, fmt::format_args args);
void vprintln(fmt::string_view fmt, fmt::format_args args);
void flush();

template <typename... Args>
void print(fmt::format_string<Args...> fmt, Args &&...args) {
  ::terminal::vprint(fmt.str, fmt::make_format_args(args...));
}

template <typename... Args>
void println(fmt::format_string<Args...> fmt, Args &&...args) {
  ::terminal::vprintln(fmt.str, fmt::make_format_args(args...));
}
} // namespace terminal

#endif
