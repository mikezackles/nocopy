#ifndef UUID_842939A6_E8D2_4609_9A12_17E3946609D6
#define UUID_842939A6_E8D2_4609_9A12_17E3946609D6

BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/set.hpp>
#include <boost/hana/type.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy { namespace detail {
  namespace hana = boost::hana;

  template <typename ...Ts>
  struct type_set {
    static constexpr auto value = hana::make_set(hana::type_c<Ts>...);
  };
}}

#endif
