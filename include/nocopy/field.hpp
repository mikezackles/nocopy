#ifndef UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E
#define UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E

#include <nocopy/fwd/field.hpp>

#ifndef NOCOPY_NO_MACROS
#define NOCOPY_FIELD(field_name, type) \
  struct field_name { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  }

#define NOCOPY_ARRAY(type, size) ::nocopy::array<type, size>
#define NOCOPY_ONEOF(...) ::nocopy::oneof<uint8_t, __VA_ARGS__>
#define NOCOPY_ONEOF16(...) ::nocopy::oneof<uint16_t, __VA_ARGS__>
#endif

#endif
