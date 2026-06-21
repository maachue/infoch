#include "base64.hpp"

#include <cstdint>
#include <span>
#include <string>

// https://github.com/fastfetch-cli/fastfetch/blob/240928138ee7cb39859b5192031943e9df30b567/src/common/impl/base64.c#L4
std::string base64_encode(std::span<const char> bytes) {
  constexpr const char kBase64Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "abcdefghijklmnopqrstuvwxyz"
                                          "0123456789"
                                          "+/";
  std::string str;
  str.reserve(10 + (bytes.size() * 4 / 3));

  std::uint32_t fbytes = 0;
  const auto *curr = bytes.data();
  const auto *ends = curr + (bytes.size() - bytes.size() % 3);
  while (curr != ends) {
    fbytes =
#ifndef _MSC_VER
        __builtin_bswap32(*(std::uint32_t *)curr);
#else
        _byteswap_ulong(*(std::uint32_t *)curr);
#endif
    str += kBase64Charset[(fbytes >> 26) & 63];
    str += kBase64Charset[(fbytes >> 20) & 63];
    str += kBase64Charset[(fbytes >> 14) & 63];
    str += kBase64Charset[(fbytes >> 8) & 63];

    curr += 3;
  }

  if (bytes.size() % 3 == 1) {

    uint64_t n = (uint64_t)*curr << 16;
    str += kBase64Charset[(n >> 18) & 63];
    str += kBase64Charset[(n >> 12) & 63];
    str += "==";
  } else if (bytes.size() % 3 == 2) {
    uint64_t n = (uint64_t)*curr++ << 16;
    n |= (uint64_t)*curr << 8;
    str += kBase64Charset[(n >> 18) & 63];
    str += kBase64Charset[(n >> 12) & 63];
    str += kBase64Charset[(n >> 6) & 63];
    str += '=';
  }

  return str;
}
