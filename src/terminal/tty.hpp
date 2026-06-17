#ifndef INFOCH_TERMINAL_TTY_H
#define INFOCH_TERMINAL_TTY_H

#include <atomic>

namespace terminal {
using term_t = int;

extern constinit std::atomic<term_t> ftty;
void open_devtty();
void close_devtty();
} // namespace terminal

#endif
