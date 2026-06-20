#ifndef INFOCH_TERMINAL_TERM_QUERY_H
#define INFOCH_TERMINAL_TERM_QUERY_H

#include <string>
#include <system_error>

namespace terminal {
enum class QueryTerminalErrCode {
  CannotWriteToTTY,
  CannotPollingTTY,
  CannotParseInput,
};

class QueryTerminalErrCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "QueryTerminalErr"; }

  std::string message(int ev) const noexcept override {
    switch (static_cast<QueryTerminalErrCode>(ev)) {
      using enum QueryTerminalErrCode;

    case CannotWriteToTTY:
      return "Cannot write to terminal";
    case CannotPollingTTY:
      return "Cannot poll terminal";
    case CannotParseInput:
      return "Cannot parse terminal input";
    }
  }
};

QueryTerminalErrCategory const &queryterminalerr_category() noexcept;

inline std::error_code make_error_code(QueryTerminalErrCode e) noexcept {
  return {static_cast<int>(e), queryterminalerr_category()};
}

/// NOTE: This API from: `ffGetTerminalRespone` from
/// [fastfetch](https://github.com/fastfetch-cli/fastfetch/blob/dev/src/common/io.h)
int query_terminal(std::error_code &err, std::string_view query, int nparams,
                   const char *pattern, ...);
} // namespace terminal

namespace std {
template <>
struct is_error_code_enum<terminal::QueryTerminalErrCode> : true_type {};
} // namespace std

#endif
