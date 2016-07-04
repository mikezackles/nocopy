#ifndef UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC
#define UUID_B2554911_1DFB_4DEE_B5A3_ECF03B2DDABC

#include "type_set.hpp"

#include "ignore_warnings_from_dependencies.hpp"
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/set.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/core/when.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

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
      static constexpr auto allowed_types =
        detail::type_set<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float>::value;
      static_assert(
        is_datapack<base_type>::value || hana::contains(allowed_types, hana::type_c<base_type>)
      , "unsupported field type"
      );
      static_assert(
        !std::is_floating_point<base_type>::value || std::numeric_limits<float>::is_iec559
      , "must use IEC 559 to enable floating point support"
      );
      using return_type = find_return_type_t<field_type>;
      static constexpr auto alignment = alignment_for<base_type>::result;
      static constexpr auto size = sizeof(return_type);
    };
  }
}

#endif
