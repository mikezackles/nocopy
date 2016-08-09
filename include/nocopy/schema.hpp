#ifndef UUID_711399AD_853A_450B_8DC0_FDBF671931EA
#define UUID_711399AD_853A_450B_8DC0_FDBF671931EA

#include <nocopy/fwd/datapack.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/filter.hpp>
#include <boost/hana/tuple.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy {
  namespace hana = boost::hana;

  template <template <typename ...> class Result, std::size_t Version, typename ...VersionRanges>
  struct schema {
  private:
    static constexpr auto fields = hana::make_tuple(hana::type_c<VersionRanges>...);
    struct field_is_in_range {
      template <typename T>
      constexpr auto operator()(T t) const {
        using field = typename decltype(t)::type;
        return hana::bool_c<field::first_version <= Version && Version <= field::last_version>;
      }
    };
    static constexpr auto current_fields = hana::filter(fields, field_is_in_range{});

    struct get_type {
      template <typename ...Ts>
      constexpr auto operator()(Ts...) const {
        return hana::type_c<Result<typename Ts::type::field...>>;
      }
    };

  public:
    using type = typename decltype(hana::unpack(current_fields, get_type{}))::type;
  };

  template <template <typename ...> class Result, std::size_t Version, typename ...VersionRanges>
  using schema_t = typename schema<datapack, Version, VersionRanges...>::type;
}

#endif