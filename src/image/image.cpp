#include "image.hpp"

#include <cstdint>
#include <exception>
#include <utility>

#include "image/fn.hpp"
#include "image/types.hpp"
#include "settings/image.hpp"
#include "terminal/io.hpp"

namespace image {
void print_image(const settings::Image &set, std::uint16_t &curr_x,
                 std::uint16_t &curr_y, std::exception_ptr &err) noexcept {
  err = nullptr;

  try {
    if (set.type == ImageType::Disable) {
      return;
    }

    if (set.padding_height != 0) {
      terminal::print("\x1b[{}B", set.padding_height);
      curr_y += set.padding_height;
    }

    if (set.padding_width != 0) {
      terminal::print("\x1b[{}C", set.padding_width);
      curr_x += set.padding_width;
    }

    size_t width = 0;
    size_t height = 0;

    if (set.type == ImageType::Auto) {
    }

    {
      std::uint16_t cwidth = set.cell_width;
      std::uint16_t cheight = set.cell_height;
      auto tmp = internal::get_size_from_cell_size(set.path, cwidth, cheight);

      curr_x += cwidth;
      curr_y += cheight;

      if (std::to_underlying(set.type) < std::to_underlying(ImageType::Chafa)) {
        width = std::get<0>(tmp);
        height = std::get<1>(tmp);
      } else {
        width = cwidth;
        height = cheight;
      }
    }

    // print ruler
    constexpr int kSizeRuler = 25;
    if constexpr (kSizeRuler != 0) {
      curr_x += 1;
      curr_y += 1;

      int i = 1;
      char c = '0';
      for (; i <= kSizeRuler; ++i) {
        terminal::print("{}", c++);
      }
      terminal::print("\n");
      c = '0';
      for (i = 1; i <= kSizeRuler; ++i) {
        if (set.padding_width != 0) {
          terminal::print("\x1b[{}C", set.padding_width);
        }
        terminal::print("{}\n", c++);
      }
      terminal::print("\x1b[{}A", kSizeRuler);
      if (set.padding_width != 0) {
        terminal::print("\x1b[{}C", set.padding_width);
      }
      terminal::print("\x1b[{}C", 1);
      terminal::flush();
    }

    switch (set.type) {
    case ImageType::Kitty:
      image::internal::kitty_print_image(set.path, width, height);
      break;
    case ImageType::Iterm:
      image::internal::iterm_print_image(set.path, width, height);
      break;
    default:
      terminal::println("huh? Calc: {}x{}", width, height);
      break;
    }
  } catch (...) {
    err = std::current_exception();
    return;
  }
}
} // namespace image
