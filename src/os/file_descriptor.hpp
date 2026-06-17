#ifndef INFOCH_OS_FILE_DESC_H
#define INFOCH_OS_FILE_DESC_H

#include <unistd.h>

#include <utility>

namespace os {
class FdPP {
  int fd_ = -1;

public:
  FdPP() = default;
  FdPP(int &fd) noexcept : fd_(fd) { fd = -1; }
  FdPP(int fd) noexcept : fd_(fd) {}

  ~FdPP() noexcept {
    if (fd_ != -1) {
      close(fd_);
    }
  }

  [[nodiscard]] int get() const { return fd_; } // for system API
  [[nodiscard]] bool is_valid() const { return fd_ != -1; }
  int release() { return std::exchange(fd_, -1); }

  // maybe helpful?
  operator bool() const { return this->is_valid(); }

  // no copy
  FdPP(FdPP &) = delete;
  FdPP &operator=(FdPP &) = delete;

  FdPP(FdPP &&lhs) noexcept { this->fd_ = std::exchange(lhs.fd_, -1); }
  FdPP &operator=(FdPP &&lhs) noexcept {
    if (this->fd_ != -1) {
      close(fd_);
    }

    this->fd_ = std::exchange(lhs.fd_, -1);
    return *this;
  }
};
} // namespace os

#endif
