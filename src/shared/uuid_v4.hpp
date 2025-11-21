//
// Created by 张文兵 on 2025/11/21.
//

#ifndef UUID_V4_HPP
#define UUID_V4_HPP

#include <array>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace shared {

inline std::string uuid_v4() {
  std::mt19937_64 rng{std::random_device{}()};
  std::array<unsigned char, 16> bytes{};
  for (int i = 0; i < 16; ++i)
    bytes[i] = static_cast<unsigned char>(rng() & 0xFF);

  // set version to 4 --- xxxx0100
  bytes[6] = (bytes[6] & 0x0F) | 0x40;
  // set variant to 10xxxxxx
  bytes[8] = (bytes[8] & 0x3F) | 0x80;

  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  for (int i = 0; i < 16; ++i) {
    oss << std::setw(2) << static_cast<int>(bytes[i]);
    if (i == 3 || i == 5 || i == 7 || i == 9)
      oss << '-';
  }
  return oss.str();
}
}  // namespace shared
#endif  // UUID_V4_HPP
