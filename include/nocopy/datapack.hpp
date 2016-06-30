#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include "detail/field_accessors.hpp"
#include "static_asserts.hpp"

#include <cassert>
#include <climits>

#include "detail/ignore_warnings_from_dependencies.hpp"
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <boost/hana/at_key.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/sort.hpp>
#include <boost/hana/zip_shortest.hpp>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy {
  namespace hana = boost::hana;

  template <typename ...Fields>
  class datapack {
    static constexpr auto types = hana::make_tuple(hana::type_c<Fields>...);

    struct alignment_is_larger {
      template <typename X, typename Y>
      constexpr auto operator()(X, Y) {
        constexpr bool result = X::type::alignment > Y::type::alignment;
        return hana::bool_c<result>;
      }
    };
    static constexpr auto fields_by_alignment = hana::sort(types, alignment_is_larger{});

    struct offset_fold_impl {
      template <typename Tuple, typename T>
      constexpr auto operator()(Tuple&& tup, T) {
        return hana::append(tup, hana::back(tup) + T::type::size);
      }
    };
    static constexpr auto offsets = hana::fold_left(
      fields_by_alignment, hana::make_tuple(std::uintptr_t{0}), offset_fold_impl{}
    );

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

    template <typename T>
    static constexpr auto get_offset() {
      constexpr auto lookup = field_offset_lookup;
      return lookup[hana::type_c<T>];
    }

    static constexpr auto max_alignment = decltype(+hana::front(fields_by_alignment))::type::alignment;

    static constexpr auto align_to(std::uintptr_t offset, std::size_t alignment) {
      return offset + (((~offset) + 1) & (alignment - 1));
    }

    using align_type = typename std::aligned_storage<max_alignment, max_alignment>::type;
    static_assert(sizeof(align_type) == max_alignment, "type used for alignment must be exactly the max alignment in size");

  public:
    static constexpr auto alignment = max_alignment;
    static constexpr auto packed_size = align_to(hana::back(offsets), alignment);
    static_assert(packed_size % alignment == 0, "");

    template <typename Field>
    decltype(auto) get() {
      assert(reinterpret_cast<std::uintptr_t>(this) % alignment == 0);
      return detail::field_getter<typename Field::return_type>::get(
        *reinterpret_cast<typename Field::return_type*>(
          reinterpret_cast<std::uintptr_t>(&buffer_) + get_offset<Field>()
        )
      );
    }

    template <typename Field>
    decltype(auto) get() const {
      assert(reinterpret_cast<std::uintptr_t>(this) % alignment == 0);
      return detail::field_getter<typename Field::return_type const>::get(
        *reinterpret_cast<typename Field::return_type const*>(
          reinterpret_cast<std::uintptr_t>(&buffer_) + get_offset<Field>()
        )
      );
    }

    template <typename Field>
    void set(typename Field::return_type val) {
      assert(reinterpret_cast<std::uintptr_t>(this) % alignment == 0);
      detail::field_setter<typename Field::return_type>::set(
        *reinterpret_cast<typename Field::return_type*>(
          reinterpret_cast<std::uintptr_t>(&buffer_) + get_offset<Field>()
        )
      , val
      );
    }

  private:
    std::array<align_type, packed_size / alignment> buffer_;
  };
}

#endif
