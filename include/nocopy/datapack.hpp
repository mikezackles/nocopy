#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include <nocopy/endianness.hpp>

#include <cassert>
#include <climits>
#include <boost/hana/at_key.hpp>
#include <boost/hana/back.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/set.hpp>
#include <boost/hana/sort.hpp>
#include <boost/hana/zip_shortest.hpp>
#include <iostream>

namespace nocopy {
  static_assert(CHAR_BIT == 8, "char must have 8 bits");
  // AFAIK the standard guarantees that std::array is POD, but we do our
  // best to make sure.
  static_assert(std::is_pod<std::array<int, 42>>::value, "std::array must be POD");
  static_assert(sizeof(std::array<int, 42>) == sizeof(int) * 42, "std::array must be POD");

  namespace hana = boost::hana;

  struct single_field {};
  template <std::size_t Count>
  struct multi_field {
    static_assert(Count > 0, "multi field must have count greater than 0");
  };

  template <typename T, std::size_t Size>
  class arraypack;

  namespace detail {
    template <typename ...Ts>
    struct set {
      static constexpr auto value = hana::make_set(hana::type_c<Ts>...);
    };

    template <typename T>
    struct is_scalar_or_floating_point
      : std::integral_constant<bool, std::is_scalar<T>::value || std::is_floating_point<T>::value> {};

    template <typename T, typename = hana::when<true>>
    struct field_getter {
      static T& get(T& val) { return val; }
    };
    template <typename T>
    struct field_getter<T, hana::when<is_scalar_or_floating_point<T>::value>> {
      static auto get(T val) { return byte_swap_if_big_endian(val); }
    };

    template <typename T> struct fail { static constexpr bool value = false; };
    template <typename T, typename = hana::when<true>>
    struct field_setter {
      static_assert(fail<T>::value, "cannot set aggregate field");
    };
    template <typename T>
    struct field_setter<T, hana::when<is_scalar_or_floating_point<T>::value>> {
      static auto set(T& target, T val) { target = byte_swap_if_big_endian(val); }
    };

    template <typename T, typename FieldType, typename = hana::when<true>>
    struct multi {};
    template <typename T>
    struct multi<T, single_field, hana::when<true>> {
      using type = T;
    };
    template <typename T, std::size_t Count>
    struct multi<T, multi_field<Count>, hana::when<hana::contains(set<uint8_t, int8_t, unsigned char, char>::value, hana::type_c<T>)>> {
      using type = std::array<T, Count>;
    };
    template <typename T, std::size_t Count>
    struct multi<T, multi_field<Count>, hana::when<!hana::contains(set<uint8_t, int8_t, unsigned char, char>::value, hana::type_c<T>)>> {
      using type = arraypack<T, Count>;
    };
    template <typename T, typename FieldType>
    using multi_t = typename multi<T, FieldType>::type;
  }

  template <typename T, typename FieldType = single_field>
  struct field {
    static_assert(sizeof(T) <= 32, "no scalar types with size greater than 32 for now");
    static constexpr auto allowed_types =
      detail::set<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float>::value;
    static_assert(hana::contains(allowed_types, hana::type_c<T>), "unsupported field type");
    static_assert(
      !std::is_floating_point<T>::value || std::numeric_limits<float>::is_iec559
    , "must use IEC 559 floating point"
    );
    using return_type = detail::multi_t<T, FieldType>;
    static constexpr auto name = "unnamed";
    static constexpr std::size_t alignment = sizeof(T);
    static constexpr std::size_t size = sizeof(return_type);
  };

  template <typename T, std::size_t Size>
  class arraypack {
  public:
    T get(std::size_t index) const { return byte_swap_if_big_endian(buffer_[index]); }
    void set(std::size_t index, T val) { buffer_[index] = byte_swap_if_big_endian(val); }
  private:
    std::array<T, Size> buffer_;
  };

  template <uint32_t Version, typename ...Fields>
  class datapack {
    static constexpr auto types = hana::make_tuple(hana::type_c<Fields>...);

    static constexpr std::uintptr_t version_offset = sizeof(uint32_t);

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

    datapack() {
      assert(reinterpret_cast<std::uintptr_t>(this) % sizeof(uint32_t) == 0);
    }

    template <typename Field>
    decltype(auto) get() {
      return detail::field_getter<typename Field::return_type>::get(
        *reinterpret_cast<typename Field::return_type*>(
          reinterpret_cast<std::uintptr_t>(&buffer_) + get_offset<Field>()
        )
      );
    }

    template <typename Field>
    decltype(auto) get() const {
      return detail::field_getter<typename Field::return_type>::get(
        *reinterpret_cast<typename Field::return_type const*>(
          reinterpret_cast<std::uintptr_t>(&buffer_) + get_offset<Field>()
        )
      );
    }

    template <typename Field>
    void set(typename Field::return_type val) {
      detail::field_setter<typename Field::return_type>::set(
        *reinterpret_cast<typename Field::return_type*>(
          reinterpret_cast<std::uintptr_t>(&buffer_) + get_offset<Field>()
        )
      , val
      );
    }

  private:
    static_assert(sizeof(uint32_t) < alignof(std::max_align_t), "maximum alignment must be greater than sizeof(uint32_t)");
    std::aligned_storage<packed_size, sizeof(uint32_t)> buffer_;
  };
}

#ifndef NOCOPY_NO_MACROS
  #define NOCOPY_FIELD(field_name, type) \
    struct field_name : ::nocopy::field<type> { \
      static constexpr auto name = #field_name; \
    }

  #define NOCOPY_ARRAY(field_name, type, size) \
    struct field_name : ::nocopy::field<type, ::nocopy::multi_field<size>> { \
      static constexpr auto name = #field_name; \
    }
#endif

#endif
