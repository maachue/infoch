#ifndef INFOCH_TERMINAL_TYPES_H
#define INFOCH_TERMINAL_TYPES_H

#include <atomic>

#define INFOCH_TERMINAL_STREAM int

namespace terminal {
extern std::atomic<bool> is_init;
extern std::atomic<INFOCH_TERMINAL_STREAM> ftty;
} // namespace terminal

#endif
