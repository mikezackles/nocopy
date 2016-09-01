#ifndef UUID_711399AD_853A_450B_8DC0_FDBF671931EA
#define UUID_711399AD_853A_450B_8DC0_FDBF671931EA

#include <nocopy/fwd/structpack.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/filter.hpp>
#include <boost/hana/tuple.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy {
  namespace detail {
    namespace hana = boost::hana;

    template <
      std::size_t Version
    , typename ...VersionRanges>
    struct versioned_structpack {
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
          return hana::type_c<structpack<typename Ts::type::field...>>;
        }
      };

    public:
      using type = typename decltype(hana::unpack(current_fields, get_type{}))::type;
    };
  }

  // NOTE - it would be simpler to inherit from the structpack type instead of
  // delegating, but as of C++14 this would not technically be an aggregate
  // type. This changes with C++17.
  template <std::size_t Version, typename ...VersionRanges>
  struct schema {
    using delegate_type = typename detail::versioned_structpack<Version, VersionRanges...>::type;

    static constexpr auto alignment() { return delegate_type::alignment(); }

    template <typename Field>
    static constexpr bool has(Field f) { return delegate_type::has(f); }

    template <template <std::size_t> class Field>
    static constexpr bool has() { return delegate_type::has(Field<Version>{}); }

    constexpr std::size_t version() const { return Version; }

    template <typename Field>
    auto const& operator[](Field f) const { return data[f]; }

    template <typename Field>
    auto& operator[](Field f) { return data[f]; }

    template <template <std::size_t> class Field>
    auto const& get() const { return data[Field<Version>{}]; }

    template <template <std::size_t> class Field>
    auto& get() { return data[Field<Version>{}]; }

    delegate_type data;
  };
}

#endif
