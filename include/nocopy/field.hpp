#ifndef UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E
#define UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E

#include "box.hpp"
#include "detail/traits.hpp"
#include "static_asserts.hpp"

namespace nocopy {
  template <typename T, std::size_t Count>
  struct multi {
    static_assert(Count > 0, "multi field must have count greater than 0");
  };

  //template <typename T>
  //struct field {
  //  //using type = T;
  //  //using return_type = detail::find_return_type_t<T>;
  //  //static constexpr auto name() { return "unnamed"; }
  //  //static constexpr std::size_t alignment() { return detail::find_alignment_c<FieldType>; }
  //  //static constexpr std::size_t size() { return sizeof(return_type); }
  //};
}

#ifndef NOCOPY_NO_MACROS
#define NOCOPY_FIELD(field_name, type) \
  struct field_name { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  }

#define NOCOPY_ARRAY(field_name, type, size) \
  struct field_name { \
    using field_type = ::nocopy::multi<type, size>; \
    static constexpr auto name() { return #field_name; } \
  }
#endif

#endif
