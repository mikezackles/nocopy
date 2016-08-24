#ifndef UUID_F3F9B105_8D0A_4259_92E3_ED39E64B09A5
#define UUID_F3F9B105_8D0A_4259_92E3_ED39E64B09A5

#include <type_traits>

namespace nocopy { namespace detail {
  template <typename To, typename From>
  constexpr std::enable_if_t<(sizeof(From) >= sizeof(To)) && std::is_integral<To>::value && std::is_integral<From>::value, To>
  narrow_cast(From from) {
    return static_cast<To>(
      static_cast<std::make_unsigned_t<From>>(from) & (static_cast<std::make_unsigned_t<To>>(-1))
    );
  }

  template <typename To, typename From>
  constexpr std::enable_if_t<sizeof(From) < sizeof(To) && std::is_integral<To>::value && std::is_integral<From>::value, To>
  narrow_cast(From from) {
    return from;
  }
}}

#endif
