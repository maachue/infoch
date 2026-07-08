#include "tty.hpp"

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <atomic>
#include <stdexcept>
#include <system_error>

// #include <fmt/base.h>

namespace terminal {
#ifndef _WIN32
constinit std::atomic<term_t> ftty{-1};

void open_devtty() {
  if (ftty.load() != -1) {
    throw std::runtime_error("(open_devtty) DO NOT CALL THIS FUNCTION TWICE");
  }

  int fd = ::open("/dev/tty", O_CLOEXEC | O_NOCTTY | O_RDWR);
  if (fd == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "failed to open \"/dev/tty\"");
  }
  // fmt::println(stderr, "open /dev/tty");
  ftty.store(fd);
}

void close_devtty() {
  int fd = ftty.load();
  if (fd != -1) {
    // fmt::println(stderr, "close /dev/tty");
    ::close(fd);
    ftty.store(-1); // backup!
  }
}
#else

constinit std::atomic<term_t> termout_handle{nullptr};
constinit std::atomic<term_t> termin_handle{nullptr};

void open_devtty() {
  if (termout_handle.load() != nullptr || termin_handle.load() != nullptr) {
    throw std::runtime_error("(open_devtty) DO NOT CALL THIS FUNCTION TWICE");
  }

  HANDLE termout_h = ::CreateFileW(L"CONOUT$", GENERIC_WRITE | GENERIC_READ,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (termout_h == INVALID_HANDLE_VALUE) {
    throw std::system_error(static_cast<int>(::GetLastError()),
                            std::system_category(),
                            "(open_devtty) failed to open CONOUT$");
  }

  HANDLE termin_h = ::CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_WRITE | FILE_SHARE_READ, 0,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (termin_h == INVALID_HANDLE_VALUE) {
    ::CloseHandle(termout_h);
    throw std::system_error(static_cast<int>(::GetLastError()),
                            std::system_category(),
                            "(open_devtty) failed to open CONIN$");
  }

  termout_handle.store(termout_h);
  termin_handle.store(termin_h);
}

void close_devtty() {
  HANDLE termout_h = termout_handle.exchange(nullptr);
  HANDLE termin_h = termin_handle.exchange(nullptr);

  if (termout_h != nullptr) {
    ::CloseHandle(termout_h);
  }

  if (termin_h != nullptr) {
    ::CloseHandle(termin_h);
  }
}
#endif
} // namespace terminal
