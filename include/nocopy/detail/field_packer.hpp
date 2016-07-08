#ifndef UUID_68BDB6D9_BB11_4462_9187_4949324917AE
#define UUID_68BDB6D9_BB11_4462_9187_4949324917AE

#include <nocopy/detail/align_to.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/at_key.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/sort.hpp>
#include <boost/hana/zip_shortest.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy { namespace detail {
  namespace hana = boost::hana;

  template <typename ...Fields>
  struct field_packer {
  private:
    static constexpr auto fields() { return hana::make_tuple(hana::type_c<Fields>...); }

    struct alignment_is_larger {
      template <typename X, typename Y>
      constexpr auto operator()(X, Y) {
        constexpr bool result = X::type::alignment > Y::type::alignment;
        return hana::bool_c<result>;
      }
    };
    static constexpr auto fields_by_alignment = hana::sort(fields(), alignment_is_larger{});

    static constexpr auto max_offset = std::numeric_limits<std::size_t>::max();
    struct offset_fold_impl {
      template <typename Pair, typename T>
      constexpr auto operator()(Pair&& pair, T) const {
        auto& tup = hana::first(pair);
        auto last_offset = hana::second(pair);
        if (max_offset - last_offset >= T::type::size) {
          auto next_offset = last_offset + T::type::size;
          return hana::make_pair(hana::append(tup, next_offset), next_offset);
        } else {
          // Avoid overflow
          return hana::make_pair(hana::append(tup, max_offset), max_offset);
        }
      }
    };
    static constexpr auto offsets_and_size = hana::fold_left(
      fields_by_alignment
    , hana::make_pair(hana::make_tuple(std::size_t{0}), std::size_t{0})
    , offset_fold_impl{}
    );
    static constexpr auto offsets = hana::first(offsets_and_size);
    static constexpr auto unaligned_size = hana::second(offsets_and_size);
    // Detect overflow - technically this could fail erroneously if the size of
    // the archive is *exactly* std::size_t
    static_assert(unaligned_size < max_offset
    , "this field pack is too large to be addressable on this machine");

    static constexpr auto field_offset_pairs = hana::zip_shortest(fields_by_alignment, offsets);

    struct lookup_fold_impl {
      template <typename Map, typename T>
      constexpr auto operator()(Map&& map, T pair) {
        return hana::insert(map, hana::make_pair(hana::front(pair), hana::back(pair)));
      }
    };
    static constexpr auto field_offset_lookup = hana::fold_left(
      field_offset_pairs
    , hana::make_map()
    , lookup_fold_impl{}
    );

    static constexpr auto max_alignment = decltype(+hana::front(fields_by_alignment))::type::alignment;

  public:
    static constexpr auto alignment = max_alignment;
    static constexpr auto packed_size = align_to(unaligned_size, alignment);
    static_assert(packed_size % alignment == 0, "");

    template <typename Field>
    static constexpr bool has() {
      return hana::contains(fields(), hana::type_c<Field>);
    }

    template <typename T>
    static constexpr auto get_offset() {
      constexpr auto lookup = field_offset_lookup;
      return lookup[hana::type_c<T>];
    }
  };
}}

#endif
