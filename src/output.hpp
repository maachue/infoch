#ifndef INFOCH_OUTPUT_H
#define INFOCH_OUTPUT_H

#include <cstdio>
#include <fmt/base.h>
#include <string_view>

#include <fmt/core.h>
#include <fmt/os.h>

namespace output {
namespace internal {}

template <typename... Args>
void print(fmt::format_string<Args...> fmt, Args &&...args) {}
} // namespace output

#endif
