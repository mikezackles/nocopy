#ifndef UUID_B8B49E62_BE30_40EA_88CA_8E3BA1EC022C
#define UUID_B8B49E62_BE30_40EA_88CA_8E3BA1EC022C

#include "endianness.hpp"

#include "ignore_warnings_from_dependencies.hpp"
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/core/when.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy { namespace detail {
  namespace hana = boost::hana;

  template <typename T, typename = hana::when<true>>
  struct field_getter {
    static T& get(T& val) { return val; }
  };
  template <typename T>
  struct field_getter<T, hana::when<std::is_scalar<T>::value>> {
    static auto get(T val) { return detail::byte_swap_if_big_endian(val); }
  };

  template <typename T> struct fail { static constexpr bool value = false; };
  template <typename T, typename = hana::when<true>>
  struct field_setter {
    static_assert(fail<T>::value, "cannot set aggregate field");
  };
  template <typename T>
  struct field_setter<T, hana::when<std::is_scalar<T>::value>> {
    static auto set(T& target, T val) { target = detail::byte_swap_if_big_endian(val); }
  };
}}

#endif
