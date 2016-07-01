#ifndef UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E
#define UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E

#include "arraypack.hpp"
#include "detail/traits.hpp"
#include "static_asserts.hpp"

#include "detail/ignore_warnings_from_dependencies.hpp"
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/set.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/core/when.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy {
  namespace hana = boost::hana;

  struct single_field {};
  template <std::size_t Count>
  struct multi_field {
    static_assert(Count > 0, "multi field must have count greater than 0");
  };

  template <typename T, typename FieldType = single_field>
  struct field {
  private:
    static constexpr auto allowed_types =
      detail::type_set<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float>::value;
    static_assert(
      detail::is_datapack<T>::value || detail::is_arraypack<T>::value || hana::contains(allowed_types, hana::type_c<T>)
    , "unsupported field type"
    );
    static_assert(
      !std::is_floating_point<T>::value || std::numeric_limits<float>::is_iec559
    , "must use IEC 559 to enable floating point support"
    );
  public:
    using return_type = detail::find_return_type_t<T, FieldType>;
    static constexpr auto name() { return "unnamed"; }
    static constexpr std::size_t alignment() { return detail::find_alignment<T>::result; }
    static constexpr std::size_t size() { return sizeof(return_type); }
  };
}

#ifndef NOCOPY_NO_MACROS
#define NOCOPY_FIELD(field_name, type) \
  struct field_name : ::nocopy::field<type> { \
    static constexpr auto name() { return #field_name; } \
  }

#define NOCOPY_ARRAY(field_name, type, size) \
  struct field_name : ::nocopy::field<type, ::nocopy::multi_field<size>> { \
    static constexpr auto name() { return #field_name; } \
  }
#endif

#endif
