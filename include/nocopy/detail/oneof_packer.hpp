#ifndef UUID_1CAA534B_0EB4_489D_8FA7_D5343EB58DD5
#define UUID_1CAA534B_0EB4_489D_8FA7_D5343EB58DD5

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/tuple.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy { namespace detail {
  template <typename Tag, typename ...Ts>
  class oneof_packer {
    static constexpr auto allowed_types = hana::make_tuple(hana::type_c<detail::field_traits<Ts>>...);

    struct alignment_ordering {
      template <typename X, typename Y>
      constexpr auto operator()(X, Y) const {
        constexpr bool result = X::type::alignment < Y::type::alignment;
        return hana::bool_c<result>;
      }
    };
    static constexpr auto payload_alignment = hana::maximum(allowed_types, alignment_ordering{});

    static constexpr auto payload_offset() { return align_to(sizeof(Tag), payload_alignment()); }

  public:
    template <typename T>
    static constexpr bool is_allowed() {
      return hana::contains(allowed_types, hana::type_c<T>);
    }

    static constexpr auto lookup_type(Tag offset) {
      return hana::at(allowed_types, offset);
    }

    static constexpr auto alignment() {
      return std::max(payload_alignment, sizeof(Tag));
    }

    static constexpr auto size() {
      return align_to(payload_offset() + payload_alignment, alignment());
    }
  };
}}

#endif
