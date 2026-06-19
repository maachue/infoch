#ifndef INFOCH_BASE64_H
#define INFOCH_BASE64_H

#include <span>
#include <string>

std::string base64_encode(std::span<const char> bytes);

#endif
