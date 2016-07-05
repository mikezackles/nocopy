#ifndef UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC
#define UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC

#include "ignore_warnings_from_dependencies.hpp"
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/core/when.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

#include <cstdint>
#include <type_traits>

namespace nocopy {
  template <typename ...Ts>
  class datapack;

  template <typename T>
  class box;

  template <typename T, std::size_t Count>
  struct multi;

  namespace detail {
    namespace hana = boost::hana;

    template <typename T>
    struct is_datapack {
      static constexpr bool value = false;
    };

    template <typename ...Ts>
    struct is_datapack<datapack<Ts...>> {
      static constexpr bool value = true;
    };

    template <typename T, typename = hana::when<true>>
    struct boxer {
      using type = T;
    };
    template <typename T>
    struct boxer<T, hana::when<std::is_scalar<T>::value && (sizeof(T) > 1)>> {
      using type = box<T>;
    };
    template <typename T>
    using boxer_t = typename boxer<T>::type;

    template <typename T>
    struct find_return_type {
      using type = boxer_t<T>;
    };
    template <typename FieldType, std::size_t Count>
    struct find_return_type<multi<FieldType, Count>> {
      using type = std::array<typename find_return_type<FieldType>::type, Count>;
    };
    template <typename NestedFieldType, std::size_t NestedCount, std::size_t Count>
    struct find_return_type<multi<multi<NestedFieldType, NestedCount>, Count>> {
      using nested = find_return_type<multi<NestedFieldType, NestedCount>>;
      using type = std::array<typename nested::type, Count>;
    };
    template <typename FieldType>
    using find_return_type_t = typename find_return_type<FieldType>::type;

    template <typename T>
    struct find_base_type {
      using type = T;
    };
    template <typename FieldType, std::size_t Count>
    struct find_base_type<multi<FieldType, Count>> {
      using type = typename find_base_type<FieldType>::type;
    };
    template <typename NestedFieldType, std::size_t NestedCount, std::size_t Count>
    struct find_base_type<multi<multi<NestedFieldType, NestedCount>, Count>> {
      using nested = find_base_type<multi<NestedFieldType, NestedCount>>;
      using type = typename nested::type;
    };
    template <typename FieldType>
    using find_base_t = typename find_base_type<FieldType>::type;

    template <typename T, typename = hana::when<true>>
    struct alignment_for {
      static constexpr std::size_t result = T::alignment();
    };
    template <typename T>
    struct alignment_for<T, hana::when<std::is_scalar<T>::value>> {
      static constexpr std::size_t result = sizeof(T);
    };

    template <typename Field>
    struct field_traits {
      using field_type = typename Field::field_type;
      using base_type = find_base_t<field_type>;
      static_assert(
           is_datapack<base_type>::value
        || (std::is_same<base_type, float>::value && std::numeric_limits<float>::is_iec559 && sizeof(float) == 4)
        || (std::is_same<base_type, double>::value && std::numeric_limits<double>::is_iec559 && sizeof(double) == 8)
      #ifdef INT8_MAX
        || std::is_same<base_type, int8_t>::value
      #endif
      #ifdef UINT8_MAX
        || std::is_same<base_type, uint8_t>::value
      #endif
      #ifdef INT16_MAX
        || std::is_same<base_type, int16_t>::value
      #endif
      #ifdef UINT16_MAX
        || std::is_same<base_type, uint16_t>::value
      #endif
      #ifdef INT32_MAX
        || std::is_same<base_type, int32_t>::value
      #endif
      #ifdef UINT32_MAX
        || std::is_same<base_type, uint32_t>::value
      #endif
      #ifdef INT64_MAX
        || std::is_same<base_type, int64_t>::value
      #endif
      #ifdef UINT64_MAX
        || std::is_same<base_type, uint64_t>::value
      #endif
      , "unsupported field type"
      );
      using return_type = find_return_type_t<field_type>;
      static constexpr auto alignment = alignment_for<base_type>::result;
      static constexpr auto size = sizeof(return_type);
    };
  }
}

#endif
