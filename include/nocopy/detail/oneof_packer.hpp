#ifndef UUID_1CAA534B_0EB4_489D_8FA7_D5343EB58DD5
#define UUID_1CAA534B_0EB4_489D_8FA7_D5343EB58DD5

#include <nocopy/detail/align_to.hpp>

#include <silence_dependencies/ignore_warnings.hpp>
SD_BEGIN_IGNORE_WARNINGS
#include <boost/hana/at.hpp>
#include <boost/hana/maximum.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/tuple.hpp>
SD_END_IGNORE_WARNINGS

namespace nocopy { namespace detail {
  template <typename Tag, typename ...Ts>
  class oneof_packer {
    static constexpr auto allowed_types() {
      return hana::make_tuple(hana::type_c<Ts>...);
    }

    struct lookup_fold {
      template <typename Pair, typename T>
      constexpr auto operator()(Pair&& pair, T t) const {
        auto& map = hana::first(pair);
        auto index = hana::second(pair);
        return hana::make_pair(hana::insert(map, hana::make_pair(t, index)), index + 1);
      }
    };
    static constexpr auto index_lookup() {
      return hana::first(hana::fold_left(
        allowed_types()
      , hana::make_pair(hana::make_map(), std::size_t{0})
      , lookup_fold{}
      ));
    }

    struct alignment_ordering {
      template <typename X, typename Y>
      constexpr auto operator()(X, Y) const {
        constexpr bool result = X::type::alignment < Y::type::alignment;
        return hana::bool_c<result>;
      }
    };
    static constexpr auto payload_alignment() {
      constexpr auto biggest = hana::maximum(allowed_types(), alignment_ordering{});
      return decltype(biggest)::type::alignment;
    }

  public:
    template <typename T>
    static constexpr bool is_allowed() {
      return hana::contains(allowed_types(), hana::type_c<T>);
    }

    static constexpr std::size_t num_types() {
      return hana::size(allowed_types());
    }

    template <typename T>
    static constexpr auto tag_for_type() {
      return index_lookup()[hana::type_c<T>];
    }

    template <std::size_t Index>
    static constexpr auto lookup_trait() {
      return typename decltype(+hana::at_c<Index>(allowed_types()))::type{};
    }

    static constexpr auto payload_offset() {
      return align_to(sizeof(Tag), payload_alignment());
    }

    static constexpr auto alignment() {
      return std::max(payload_alignment(), sizeof(Tag));
    }

    static constexpr auto size() {
      return align_to(payload_offset() + payload_alignment(), alignment());
    }
  };
}}

#endif
