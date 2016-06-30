#ifndef UUID_711399AD_853A_450B_8DC0_FDBF671931EA
#define UUID_711399AD_853A_450B_8DC0_FDBF671931EA

#include "type_set.hpp"

namespace nocopy {
  template <std::size_t Version, typename ...VersionRanges>
  struct archive {
  private:
    struct field_is_in_range {
      template <typename Field>
      constexpr auto operator()(Field f) {
        using field = typename decltype(f)::type;
        constexpr bool in_range = field::first_version < Version && Version < field::last_version;
        return hana::bool_c<in_range>;
      }
    };

    static constexpr auto current_fields = hana::filter(type_set<VersionRanges...>::value, field_is_in_range{});

    template <typename ...Ts>
    static constexpr auto get_datapack(Ts...) {
      return hana::type_c<datapack<typename Ts::type...>>;
    }

  public:
    using datapack = typename decltype(hana::unpack(current_fields, &get_datapack))::type;
  };
}

#endif
