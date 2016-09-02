#ifndef UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E
#define UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E

#include <nocopy/fwd/field.hpp>

#ifndef NOCOPY_NO_MACROS
#define NOCOPY_FIELD(field_name, type) \
  struct field_name ## _t { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  }; \
  static constexpr field_name ## _t field_name{}

#define NOCOPY_PRIVATE_FIELD(field_name, type, containing_class) \
  struct field_name ## _t { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  private: \
    friend class containing_class; \
    constexpr field_name ## _t() {} \
  }; \
  static constexpr field_name ## _t field_name{}

#define NOCOPY_VERSIONED_FIELD(field_name, type) \
  template <std::size_t Version> \
  struct field_name ## _t { \
    using field_type = typename type ::v<Version>; \
    static constexpr auto name() { return #field_name; } \
  }; \
  template <std::size_t Version> \
  static constexpr field_name ## _t<Version> field_name{}

#define NOCOPY_ARRAY(type, size) ::nocopy::array<type, size>
#define NOCOPY_ONEOF(...) ::nocopy::oneof8<__VA_ARGS__>
#define NOCOPY_ONEOF16(...) ::nocopy::oneof16<__VA_ARGS__>
#endif

#endif
