#ifndef UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E
#define UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E

#include "static_asserts.hpp"

namespace nocopy {
  template <typename T, std::size_t Count>
  struct array {
    static_assert(Count > 0, "array field must have count greater than 0");
  };
}

#ifndef NOCOPY_NO_MACROS
#define NOCOPY_FIELD(field_name, type) \
  struct field_name { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  }

#define NOCOPY_ARRAY(field_name, type, size) \
  struct field_name { \
    using field_type = ::nocopy::array<type, size>; \
    static constexpr auto name() { return #field_name; } \
  }
#endif

#endif
