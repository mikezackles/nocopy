#ifndef UUID_0813C949_44BD_49FF_912B_50E775E5ACF3
#define UUID_0813C949_44BD_49FF_912B_50E775E5ACF3

namespace nocopy { namespace detail {
  constexpr auto align_to(std::size_t offset, std::size_t alignment) {
    return offset + (((~offset) + 1) & (alignment - 1));
  }
}}

#endif
