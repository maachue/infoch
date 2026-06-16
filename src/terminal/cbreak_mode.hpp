#ifndef INFOCH_TERMINAL_CBREAK_MODE_H
#define INFOCH_TERMINAL_CBREAK_MODE_H

#include <system_error>

namespace terminal {
bool init_cbreak_mode(std::error_code &err) noexcept;
void deinit_mode() noexcept;
} // namespace terminal

#endif
