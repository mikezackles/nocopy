#ifndef UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC
#define UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC

#include <nocopy/fwd/box.hpp>
#include <nocopy/fwd/datapack.hpp>
#include <nocopy/fwd/field.hpp>
#include <nocopy/fwd/oneof.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/core/when.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

#include <array>
#include <climits>
#include <cstdint>
#include <type_traits>

namespace nocopy {
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

    template <typename T>
    struct is_oneof {
      static constexpr bool value = false;
    };
    template <typename ...Ts>
    struct is_datapack<oneof<Ts...>> {
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
    struct find_return_type<array<FieldType, Count>> {
      using nested = typename find_return_type<FieldType>::type;
      using type = std::array<nested, Count>;
      static_assert(std::is_pod<type>::value, "std::array must be POD");
      static_assert(sizeof(type) == sizeof(nested) * Count, "std::array contains extra data");
    };
    template <typename NestedFieldType, std::size_t NestedCount, std::size_t Count>
    struct find_return_type<array<array<NestedFieldType, NestedCount>, Count>> {
      using nested = typename find_return_type<array<NestedFieldType, NestedCount>>::type;
      using type = std::array<nested, Count>;
      static_assert(std::is_pod<type>::value, "std::array must be POD");
      static_assert(sizeof(type) == sizeof(nested) * Count, "std::array contains extra data");
    };
    template <typename FieldType>
    using find_return_type_t = typename find_return_type<FieldType>::type;

    template <typename T>
    struct find_base_type {
      using type = T;
    };
    template <typename FieldType, std::size_t Count>
    struct find_base_type<array<FieldType, Count>> {
      using type = typename find_base_type<FieldType>::type;
    };
    template <typename NestedFieldType, std::size_t NestedCount, std::size_t Count>
    struct find_base_type<array<array<NestedFieldType, NestedCount>, Count>> {
      using nested = find_base_type<array<NestedFieldType, NestedCount>>;
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

    template <typename BaseType>
    constexpr bool is_valid_base_type() {
      return
           is_datapack<BaseType>::value
        || (std::is_same<BaseType, float>::value && std::numeric_limits<float>::is_iec559 && sizeof(float) * CHAR_BIT == 32)
        || (std::is_same<BaseType, double>::value && std::numeric_limits<double>::is_iec559 && sizeof(double) * CHAR_BIT == 64)
        || std::is_same<BaseType, char>::value
        || std::is_same<BaseType, unsigned char>::value
      #ifdef INT8_MAX
        || std::is_same<BaseType, int8_t>::value
      #endif
      #ifdef UINT8_MAX
        || std::is_same<BaseType, uint8_t>::value
      #endif
      #ifdef INT16_MAX
        || std::is_same<BaseType, int16_t>::value
      #endif
      #ifdef UINT16_MAX
        || std::is_same<BaseType, uint16_t>::value
      #endif
      #ifdef INT32_MAX
        || std::is_same<BaseType, int32_t>::value
      #endif
      #ifdef UINT32_MAX
        || std::is_same<BaseType, uint32_t>::value
      #endif
      #ifdef INT64_MAX
        || std::is_same<BaseType, int64_t>::value
      #endif
      #ifdef UINT64_MAX
        || std::is_same<BaseType, uint64_t>::value
      #endif
      ;
    }

    template <typename Field>
    struct field_traits {
      using original_type = Field;
      using field_type = typename Field::field_type;
      using base_type = find_base_t<field_type>;
      static_assert(is_valid_base_type<base_type>(), "unsupported field type");
      using return_type = find_return_type_t<field_type>;
      static constexpr auto alignment = alignment_for<base_type>::result;
      static constexpr auto size = sizeof(return_type);
    };
  }
}

#endif
