#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include <nocopy/endianness.hpp>

#include <climits>
#include <boost/hana/at_key.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/set.hpp>
#include <boost/hana/sort.hpp>
#include <boost/hana/zip_shortest.hpp>
#include <iostream>
#include <optional.hpp>

namespace nocopy {
  static_assert(CHAR_BIT == 8, "char must have 8 bits");

  namespace hana = boost::hana;
  template <typename T>
  using optional = std::experimental::optional<T>;

  namespace detail {
    //template <typename T>
    //struct raw_type {
    //  using type = T;
    //};

    //template <typename T, std::size_t Size>
    //struct raw_type<std::array<T, Size>> {
    //  // AFAIK std::array is already guaranteed to be POD, but we do a few
    //  // checks just to be sure
    //  static_assert(std::is_pod_v<std::array<T, Size>>, "std::array must be POD");
    //  static_assert(sizeof(std::array<T, Size>) == Size, "std::array must be POD");
    //  using type = T;
    //};
    //template <typename T>
    //using raw_type_t = typename raw_type<T>::type;

    template <typename ...Ts>
    struct set {
      static constexpr auto value = hana::make_set(hana::type_c<Ts>...);
    };
  }

  template <typename T>
  struct field {
    static constexpr auto allowed_types =
      detail::set<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float>::value;
    static_assert(sizeof(T) <= 32, "no scalar types with size greater than 32 for now");
    static_assert(hana::contains(allowed_types, hana::type_c<T>), "unsupported field type");
    static_assert(
      !std::is_floating_point<T>::value || std::numeric_limits<float>::is_iec559
    , "must use IEC 559 floating point"
    );
    using type = T;
    static constexpr auto name = "unnamed";
    static constexpr std::size_t alignment = sizeof(T);
    static constexpr std::size_t size = sizeof(T);
  };

  template <uint32_t Version, typename ...Fields>
  class datapack {
    static constexpr auto types = hana::make_tuple(hana::type_c<Fields>...);

    static constexpr auto version_offset = sizeof(uint32_t);

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
        return hana::append(tup, hana::back(tup) + T::type::alignment);
      }
    };
    static constexpr auto offsets = hana::fold_left(
      fields_by_alignment, hana::make_tuple(version_offset), offset_fold_impl{}
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

  public:
    static constexpr auto packed_size = hana::back(offsets);

    static optional<datapack> create(void* bufferp, std::size_t size) {
      // TODO - accept std::error_code as out arg
      if (reinterpret_cast<std::uintptr_t>(bufferp) % sizeof(uint32_t) != 0) {
        std::cerr << "cannot create datapack from unaligned buffer" << std::endl;
        return {};
      } else if (size < packed_size) {
        std::cerr << "buffer is too small to hold datapack" << std::endl;
        return {};
      }
      return datapack{bufferp};
    }

    template <typename Field>
    typename Field::type get() const {
      return byte_swap_if_big_endian(
        *reinterpret_cast<typename Field::type*>(&bufferp_[get_offset<Field>()])
      );
    }

    template <typename Field>
    void set(typename Field::type val) {
      *reinterpret_cast<typename Field::type*>(bufferp_ + get_offset<Field>()) = byte_swap_if_big_endian(val);
    }
  private:
    datapack(void* bufferp) : bufferp_(reinterpret_cast<uint8_t*>(bufferp)) {
      *reinterpret_cast<uint32_t*>(bufferp_) = Version;
    }
    uint8_t* bufferp_;
  };
}

#ifndef NOCOPY_NO_MACROS
  #define NOCOPY_FIELD(field_name, type)              \
    struct field_name : ::nocopy::field<type> { \
      static constexpr auto name = #field_name; \
    }
#endif

#endif
