#define _CRT_SECURE_NO_WARNINGS

#include "image.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdint>
#include <exception>
#include <filesystem>
#include <stdexcept>

#include <Magick++.h>

#include "settings/image.hpp"

#include "image/protocol/iterm.hpp"
#include "image/protocol/kitty.hpp"
#include "image/types.hpp"
#include "image/utils.hpp"

#include "terminal/detection.hpp"
#include "terminal/io.hpp"

namespace image {
void modified_hehe(ImageType &set) {
  auto const &term = terminal::get_terminal();

  /* speific for wezterm */
  if (term.name == "wezterm") {
    set = ImageType::Iterm;
    return;
  }
  if (term.support_kitty) {
    set = std::getenv("INFOCH_IF_SUPPORTED_AUTO_KITTY_PATH_IMAGE") != nullptr
              ? ImageType::KittyPath
              : ImageType::Kitty;
  } else if (term.support_iterm) {
    set = ImageType::Iterm;
  } else if (term.support_sixel) {
    set = ImageType::Sixel;
  } else {
    set = std::getenv("INFOCH_IF_UNSUPPORTED_AUTO_DISABLE_IMAGE") != nullptr
              ? ImageType::Disable
              : ImageType::Chafa;
  }
}

void print_image(settings::Image &set, std::uint16_t &curr_x,
                 std::uint16_t &curr_y, std::exception_ptr &err) noexcept {
  err = nullptr;

  try {
    // path
    if (set.path.empty()) {
      throw std::runtime_error("(print_image) image path is empty");
    }

    {
      auto fstat = std::filesystem::status(set.path);

      if (!std::filesystem::exists(fstat)) {
        throw std::runtime_error(
            fmt::format("(print_image) image path doesn't exist: {}",
#ifdef _WIN32
                        reinterpret_cast<const char *>(
                            set.path.u8string()
                                .data()) // .c_str() on windows returned wchar *
#else
                        set.path.c_str()
#endif
                        ));
      }

      if (std::filesystem::is_directory(fstat)) {
        throw std::runtime_error(fmt::format(
            "(print_image) image path is a directory: {}",
#ifdef _WIN32
            reinterpret_cast<const char *>(set.path.u8string().data())
#else
            set.path.c_str()
#endif
                ));
      }

      set.path = std::filesystem::canonical(set.path); // expand!
    }

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
      modified_hehe(set.type);
    }

    Magick::Image image;
    {
      auto path = set.path.u8string();
      std::string p(path.begin(), path.end());
      if (set.type == ImageType::KittyPath) {
        image.ping(p);
      } else {
        image.read(p);
      }
    }

    DetailedImageSize render_image_size{.cell_width = set.cell_width,
                                        .cell_height = set.cell_height};

    internal::get_size_from_cell_size(render_image_size, image.columns(),
                                      image.rows());

    curr_x += render_image_size.cell_width;
    curr_y += render_image_size.cell_height;

    fmt::println(stderr, "BE-RESIZE: {} xx {}c @ {} xx {}px",
                 render_image_size.cell_width, render_image_size.cell_height,
                 render_image_size.pixel_width, render_image_size.pixel_height);

    if (set.type == ImageType::KittyPath) {
      internal::kitty_path_print_image(set.path, render_image_size);
      return;
    }

    image.resize(Magick::Geometry(render_image_size.pixel_width,
                                  render_image_size.pixel_height));
    render_image_size.pixel_width = image.columns();
    render_image_size.pixel_height = image.rows();

    fmt::println(stderr, "RESIZE: {} xx {}c @ {} xx {}px",
                 render_image_size.cell_width, render_image_size.cell_height,
                 render_image_size.pixel_width, render_image_size.pixel_height);

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
      image.magick("RGBA");
      break;
    case ImageType::Iterm:
      image.magick("TIFF");
      break;
    case ImageType::Sixel:
      image.magick("SIXEL");
      break;
    default:
      return;
    }

    Magick::Blob blob;
    image.write(&blob);

    switch (set.type) {
    case ImageType::Sixel: {

      terminal::print(
          "{}", std::string_view((const char *)blob.data(), blob.length()));

      terminal::flush();
      break;
    case ImageType::Kitty: {
      auto str = blob.base64();
      internal::kitty_print_image(str, render_image_size);
      break;
    }
    case ImageType::Iterm: {
      auto str = blob.base64();
      internal::iterm_print_image(str, render_image_size);
      break;
    }
    default:
      break;
    }
    }

    return;
  } catch (...) {
    err = std::current_exception();
    return;
  }
}
} // namespace image
