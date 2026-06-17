#ifndef INFOCH_TERMINAL_CBREAK_MODE_H
#define INFOCH_TERMINAL_CBREAK_MODE_H

#include <atomic>

namespace terminal {
extern constinit std::atomic<bool> is_init;

bool init_cbreak_mode();
void deinit_cbreak_mode() noexcept;
} // namespace terminal

#endif
