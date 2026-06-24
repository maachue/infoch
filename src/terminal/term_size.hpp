#ifndef INFOCH_TERMINAL_TERM_SIZE_H
#define INFOCH_TERMINAL_TERM_SIZE_H

#include <cstdint>
#include <string>
#include <system_error>
#include <type_traits>

namespace terminal {
struct TermSize {
  std::uint16_t cell_width;
  std::uint16_t cell_height;

  std::uint16_t pixel_width;
  std::uint16_t pixel_height;

  [[nodiscard]] bool is_zero() const noexcept {
    return cell_height == 0 || cell_height == 0 || pixel_height == 0 ||
           pixel_width == 0;
  }
};

extern constinit TermSize termsize;

enum class TermFetchSizeErrCode { CannotQueryPixelSize, CannotQueryCellSize };

class TermFetchSizeErrCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "TermFetchSizeErr"; }

  std::string message(int ev) const override {
    switch (static_cast<TermFetchSizeErrCode>(ev)) {
      using enum TermFetchSizeErrCode;

    case CannotQueryPixelSize:
      return "Cannot query terminal "
             "size (pixels)";
    case CannotQueryCellSize:
      return "Cannot query terminal "
             "size (cells)";
    default:
      return "??";
    }
  }
};

const std::error_category &termfetchsize_category() noexcept;

inline std::error_code make_error_code(TermFetchSizeErrCode e) noexcept {
  return {static_cast<int>(e), termfetchsize_category()};
}

void fetch_terminal_size();
} // namespace terminal

namespace std {
template <>
struct is_error_code_enum<terminal::TermFetchSizeErrCode> : true_type {};
} // namespace std

#endif
