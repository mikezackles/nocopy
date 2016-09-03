#ifndef UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC
#define UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC

#include <nocopy/detail/delegate.hpp>

#include <nocopy/fwd/box.hpp>
#include <nocopy/fwd/oneof.hpp>
#include <nocopy/fwd/structpack.hpp>
#include <nocopy/fwd/field.hpp>

#include <array>
#include <climits>
#include <cstdint>
#include <type_traits>

namespace nocopy {
  namespace detail {
    template <typename T>
    struct is_structpack : std::false_type {};
    template <typename ...Ts>
    struct is_structpack<structpack<Ts...>> : std::true_type {};

    template <typename T>
    struct is_oneof : std::false_type {};
    template <typename ...Ts>
    struct is_oneof<oneof<Ts...>> : std::true_type {};

    template <typename T, typename = void>
    struct boxer {
      using type = T;
    };
    template <typename T>
    struct boxer<T, std::enable_if_t<std::is_scalar<T>::value && (sizeof(T) > 1)>> {
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
      static_assert(sizeof(type) == sizeof(nested) * Count, "std::array contains extra data");
    };
    template <typename NestedFieldType, std::size_t NestedCount, std::size_t Count>
    struct find_return_type<array<array<NestedFieldType, NestedCount>, Count>> {
      using nested = typename find_return_type<array<NestedFieldType, NestedCount>>::type;
      using type = std::array<nested, Count>;
      static_assert(sizeof(type) == sizeof(nested) * Count, "std::array contains extra data");
    };
    template <typename FieldType>
    using find_return_type_t = typename find_return_type<FieldType>::type;

    template <typename T>
    struct find_base_type {
      using type = typename find_delegate<T>::type;
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

    template <typename T, typename = void>
    struct find_alignment {
      static constexpr std::size_t result = T::alignment();
    };
    template <typename T>
    struct find_alignment<T, std::enable_if_t<std::is_scalar<T>::value>> {
      static constexpr std::size_t result = sizeof(T);
    };

    template <typename T>
    static constexpr auto alignment_for() {
      using base = detail::find_base_t<T>;
      return detail::find_alignment<base>::result;
    }

    template <typename BaseType>
    constexpr bool is_valid_base_type() {
      return
           is_structpack<BaseType>::value
        || is_oneof<BaseType>::value
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

    template <typename T>
    static constexpr void assert_valid_type() {
      using base = detail::find_base_t<T>;
      static_assert(detail::is_valid_base_type<base>(), "Invalid type");
    }

    template <typename Field, typename = void>
    struct find_aggregate { using type = typename Field::field_type; };
    template <typename Field>
    struct find_aggregate<
      Field, typename check_exists<typename Field::aggregate_type::nocopy_type>::type
    > {
      using type = typename Field::aggregate_type::nocopy_type;
    };
    template <typename Field>
    using find_aggregate_t = typename find_aggregate<Field>::type;

    template <typename Aggregate>
    struct aggregate_traits {
      using base_type = find_base_t<Aggregate>;
      static_assert(is_valid_base_type<base_type>(), "unsupported field type");
      using return_type = find_return_type_t<Aggregate>;
      static constexpr auto alignment = detail::find_alignment<base_type>::result;
      static constexpr auto size = sizeof(return_type);

      static return_type& wrap(unsigned char& buffer) {
        return reinterpret_cast<return_type&>(buffer);
      }
      static return_type const& wrap(unsigned char const& buffer) {
        return reinterpret_cast<return_type const&>(buffer);
      }
    };

    template <typename Field>
    struct field_traits : aggregate_traits<find_aggregate_t<Field>> {
      using original_type = Field;
    };
  }
}

#endif
