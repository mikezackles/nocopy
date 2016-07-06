#ifndef UUID_0813C949_44BD_49FF_912B_50E775E5ACF3
#define UUID_0813C949_44BD_49FF_912B_50E775E5ACF3

namespace nocopy { namespace detail {
  // It should be safe to assume uintptr_t >= size_t, so we go ahead and
  // convert alignment from size_t to uintptr_t
  constexpr auto align_to(std::uintptr_t offset, std::uintptr_t alignment) {
    return offset + (((~offset) + 1) & (alignment - 1));
  }
}}

#endif
