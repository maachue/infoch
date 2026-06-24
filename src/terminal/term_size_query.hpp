#ifndef INFOCH_TERMINAL_TERM_SIZE_QUERY_H
#define INFOCH_TERMINAL_TERM_SIZE_QUERY_H

#include <cstdint>

namespace terminal {
void query_terminal_termsize_pixel(std::uint16_t &width, std::uint16_t &height);
void query_terminal_termsize_cell(std::uint16_t &width, std::uint16_t &height);
} // namespace terminal

#endif