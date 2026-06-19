#ifndef INFOCH_TERMINAL_TERM_QUERY_H
#define INFOCH_TERMINAL_TERM_QUERY_H

#include <system_error>

namespace terminal {
/// NOTE: This API from: `ffGetTerminalRespone` from
/// [fastfetch](https://github.com/fastfetch-cli/fastfetch/blob/dev/src/common/io.h)
int query_terminal(std::error_code &err, std::string_view query, int nparams,
                   const char *pattern, ...);
} // namespace terminal

#endif
