#ifndef UUID_B7B32235_91A3_4910_91DC_A00A37DCEE9C
#define UUID_B7B32235_91A3_4910_91DC_A00A37DCEE9C

#include <climits>
#include <array>

namespace nocopy {
  static_assert(CHAR_BIT == 8, "char must have 8 bits");
  // AFAIK the standard guarantees that std::array is POD, but we do our
  // best to make sure.
  static_assert(std::is_pod<std::array<int, 42>>::value, "std::array must be POD");
  static_assert(sizeof(std::array<int, 42>) == sizeof(int) * 42, "std::array must be POD");
}

#endif
