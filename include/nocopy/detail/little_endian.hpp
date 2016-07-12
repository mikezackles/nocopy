#ifndef UUID_D92CFF0B_148A_4023_B336_BE78F6E57ED0
#define UUID_D92CFF0B_148A_4023_B336_BE78F6E57ED0

#include <climits>
#include <cstring>
#include <type_traits>

namespace nocopy { namespace detail {
  constexpr bool optimize_little_endian() {
  #ifdef NOCOPY_OPTIMIZE_LITTLE_ENDIAN
    return true;
  #else
    return false;
  #endif
  }

  template <typename Scalar, typename = hana::when<true>>
  struct little_endian {
    static_assert(sizeof(Scalar) > 1 && optimize_little_endian(), "little-endian optimized code instantiated erroneously");

    using byte_array = std::array<unsigned char, sizeof(Scalar)>;
    static Scalar load(byte_array const& source) {
      Scalar result;
      std::memcpy(&result, &source[0], sizeof(Scalar));
      return result;
    }
    static void store(Scalar source, byte_array& target) {
      std::memcpy(&target[0], &source, sizeof(Scalar));
    }
  };

  template <typename Scalar, std::size_t Index>
  struct converter {
    using byte_array = std::array<unsigned char, sizeof(Scalar)>;
    static_assert(Index < sizeof(Scalar), "");
    static Scalar load(byte_array const& source) {
      return (Scalar{source[Index]} << (CHAR_BIT * Index)) | converter<Scalar, Index - 1>::load(source);
    }
    static void store(Scalar source, byte_array& target) {
      Scalar tmp = source >> (CHAR_BIT * Index);
      target[Index] = reinterpret_cast<unsigned char&>(tmp);
      converter<Scalar, Index - 1>::store(source, target);
    }
  };

  template <typename Scalar>
  struct converter<Scalar, 0> {
    using byte_array = std::array<unsigned char, sizeof(Scalar)>;
    static Scalar load(byte_array const& source) {
      return source[0];
    }
    static void store(Scalar source, byte_array& target) {
      target[0] = reinterpret_cast<unsigned char&>(source);
    }
  };

  template <typename Scalar>
  using converter_t = converter<Scalar, sizeof(Scalar) - 1>;

  template <typename Scalar>
  struct little_endian<Scalar, hana::when<!std::is_floating_point<Scalar>::value && !optimize_little_endian()>> {
    using byte_array = std::array<unsigned char, sizeof(Scalar)>;
    static Scalar load(byte_array const& source) {
      return converter_t<Scalar>::load(source);
    }
    static void store(Scalar source, byte_array& target) {
      converter_t<Scalar>::store(source, target);
    }
  };

  template <std::size_t Size> struct get_proxy;
  #ifdef UINT32_MAX
    template <> struct get_proxy<4> { using type = uint32_t; };
  #endif
  #ifdef UINT64_MAX
    template <> struct get_proxy<8> { using type = uint64_t; };
  #endif
  template <std::size_t Size> using get_proxy_t = typename get_proxy<Size>::type;

  template <typename Scalar>
  struct little_endian<Scalar, hana::when<std::is_floating_point<Scalar>::value && !optimize_little_endian()>> {
    using byte_array = std::array<unsigned char, sizeof(Scalar)>;
    using proxy_type = get_proxy_t<sizeof(Scalar)>;
    static Scalar load(byte_array const& source) {
      proxy_type tmp = little_endian<proxy_type>::load(source);
      Scalar result;
      std::memcpy(&result, &tmp, sizeof(Scalar));
      return result;
    }
    static void store(Scalar source, byte_array& target) {
      proxy_type tmp;
      std::memcpy(&tmp, &source, sizeof(Scalar));
      little_endian<proxy_type>::store(tmp, target);
    }
  };
}}

#endif
