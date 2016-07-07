#ifndef UUID_F2774B94_E4AD_4159_9230_0A991086A19E
#define UUID_F2774B94_E4AD_4159_9230_0A991086A19E

#include <nocopy/static_asserts.hpp>

namespace nocopy {
  template <typename Field, std::size_t FirstVersion, std::size_t LastVersion = std::numeric_limits<std::size_t>::max()>
  struct version_range {
    using field = Field;
    static constexpr auto first_version = FirstVersion;
    static constexpr auto last_version = LastVersion;
    static_assert(first_version < last_version
    , "field cannot be removed in a version lower than the one in which it is added");
  };
}

#endif
