#ifndef UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E
#define UUID_CA2A183A_D196_4445_B344_55ACD2FB9B0E

#include "arraypack.hpp"
#include "static_asserts.hpp"

#include <boost/hana/set.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/core/when.hpp>

namespace nocopy {
  namespace hana = boost::hana;

  struct single_field {};
  template <std::size_t Count>
  struct multi_field {
    static_assert(Count > 0, "multi field must have count greater than 0");
  };

  template <typename ...Ts>
  class datapack;

  namespace detail {
    template <typename ...Ts>
    struct type_set {
      static constexpr auto value = hana::make_set(hana::type_c<Ts>...);
    };

    template <typename T>
    struct is_datapack {
      static constexpr bool value = false;
    };

    template <typename ...Ts>
    struct is_datapack<datapack<Ts...>> {
      static constexpr bool value = true;
    };

    template <typename T>
    struct is_arraypack {
      static constexpr bool value = false;
    };

    template <typename T, std::size_t Size>
    struct is_arraypack<arraypack<T, Size>> {
      static constexpr bool value = true;
    };

    template <typename T, typename FieldType, typename = hana::when<true>>
      struct find_return_type {};
    template <typename T>
    struct find_return_type<T, single_field, hana::when<true>> {
      using type = T;
    };
    template <typename T, std::size_t Count>
    struct find_return_type<T, multi_field<Count>
                          , hana::when<hana::contains(type_set<uint8_t, int8_t, unsigned char, char>::value, hana::type_c<T>)>
      > {
      using type = std::array<T, Count>;
    };
    template <typename T, std::size_t Count>
    struct find_return_type<T, multi_field<Count>
                          , hana::when<!hana::contains(type_set<uint8_t, int8_t, unsigned char, char>::value, hana::type_c<T>)>
      > {
      using type = arraypack<T, Count>;
    };
    template <typename T, typename FieldType>
    using find_return_type_t = typename find_return_type<T, FieldType>::type;
  }

  template <typename T, typename FieldType = single_field>
  struct field {
    static constexpr auto allowed_types =
      detail::type_set<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float>::value;
    static_assert(
      detail::is_datapack<T>::value || detail::is_arraypack<T>::value || hana::contains(allowed_types, hana::type_c<T>)
    , "unsupported field type"
    );
    static_assert(
      !std::is_floating_point<T>::value || std::numeric_limits<float>::is_iec559
    , "must use IEC 559 floating point"
    );
    using return_type = detail::find_return_type_t<T, FieldType>;
    static constexpr auto name = "unnamed";
    static constexpr std::size_t alignment = sizeof(T);
    static constexpr std::size_t size = sizeof(return_type);
  };
}

#ifndef NOCOPY_NO_MACROS
#define NOCOPY_FIELD(field_name, type) \
  struct field_name : ::nocopy::field<type> { \
    static constexpr auto name = #field_name;   \
  }

#define NOCOPY_ARRAY(field_name, type, size) \
  struct field_name : ::nocopy::field<type, ::nocopy::multi_field<size>> { \
    static constexpr auto name = #field_name; \
  }
#endif

#endif
