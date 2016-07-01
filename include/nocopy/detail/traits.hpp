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

  template <typename T, std::size_t Count>
  class arraypack;

  struct single_field;

  template <std::size_t Count>
  struct multi_field;

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
    struct is_arraypack {
      static constexpr bool value = false;
    };

    template <typename T, std::size_t Size>
    struct is_arraypack<arraypack<T, Size>> {
      static constexpr bool value = true;
    };

    template <typename T>
    constexpr auto may_byte_swap() {
      return !hana::contains(type_set<uint8_t, int8_t, unsigned char, char>::value, hana::type_c<T>);
    }

    template <typename T, typename FieldType, typename = hana::when<true>>
    struct find_return_type {};
    template <typename T>
    struct find_return_type<T, single_field, hana::when<true>> {
      using type = T;
    };
    template <typename T, std::size_t Count>
    struct find_return_type<T, multi_field<Count>, hana::when<!may_byte_swap<T>()>> {
      using type = std::array<T, Count>;
    };
    template <typename T, std::size_t Count>
    struct find_return_type<T, multi_field<Count>, hana::when<may_byte_swap<T>()>> {
      using type = arraypack<T, Count>;
    };
    template <typename T, typename FieldType>
    using find_return_type_t = typename find_return_type<T, FieldType>::type;

    template <typename T, typename = hana::when<true>>
    struct find_alignment {
      static constexpr std::size_t result = sizeof(T);
    };
    template <typename T>
    struct find_alignment<T, hana::when<is_datapack<T>::value>> {
      static constexpr std::size_t result = T::alignment();
    };
  }
}

#endif
