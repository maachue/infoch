#ifndef INFOCH_TERMINAL_PASSTHROUGH_H
#define INFOCH_TERMINAL_PASSTHROUGH_H

#include <string_view>

namespace terminal {
constexpr std::string_view kTmuxPassthroughBegin = "\x1bPtmux;";
constexpr std::string_view kTmuxPassthroughEnd = "\x1b\\";

constexpr std::string_view kScreenPassthroughBegin = "\x1b[Ptmux;";
constexpr std::string_view kScreenPassthroughEnd = "\x1b\\";
} // namespace terminal

#endif
