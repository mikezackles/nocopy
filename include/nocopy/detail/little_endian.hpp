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
    static_assert(sizeof(Scalar) > 0 && optimize_little_endian(), "endian SFINAE failed sanity check");
    static_assert(CHAR_BIT == 8, "endian conversions assume an 8-bit byte");
    static_assert(1 < sizeof(Scalar) && sizeof(Scalar) <= 8
    , "byte swap not supported for this scalar");

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

  template <typename Scalar>
  struct little_endian<Scalar, hana::when<sizeof(Scalar) == 2 && !optimize_little_endian()>> {
    using byte_array = std::array<unsigned char, 2>;
    static Scalar load(byte_array const& source) {
      return
          (Scalar{source[0]} << 0)
        | (Scalar{source[1]} << 8);
    }
    static void store(Scalar source, byte_array& target) {
      Scalar tmp;
      tmp = source >> 0;
      target[0] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 8;
      target[1] = reinterpret_cast<unsigned char&>(tmp);
    }
  };

  template <typename Scalar>
  struct little_endian<Scalar, hana::when<sizeof(Scalar) == 4 && !std::is_floating_point<Scalar>::value && !optimize_little_endian()>> {
    using byte_array = std::array<unsigned char, 4>;
    static Scalar load(byte_array const& source) {
      return
          (Scalar{source[0]} << 0)
        | (Scalar{source[1]} << 8)
        | (Scalar{source[2]} << 16)
        | (Scalar{source[3]} << 24);
    }
    static void store(Scalar source, byte_array& target) {
      Scalar tmp;
      tmp = source >> 0;
      target[0] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 8;
      target[1] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 16;
      target[2] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 24;
      target[3] = reinterpret_cast<unsigned char&>(tmp);
    }
  };

  template <typename Scalar>
  struct little_endian<Scalar, hana::when<sizeof(Scalar) == 8 && !std::is_floating_point<Scalar>::value && !optimize_little_endian()>> {
    using byte_array = std::array<unsigned char, 8>;
    static Scalar load(byte_array const& source) {
      return
          (Scalar{source[0]} << 0)
        | (Scalar{source[1]} << 8)
        | (Scalar{source[2]} << 16)
        | (Scalar{source[3]} << 24)
        | (Scalar{source[4]} << 32)
        | (Scalar{source[5]} << 40)
        | (Scalar{source[6]} << 48)
        | (Scalar{source[7]} << 56);
    }
    static void store(Scalar source, byte_array& target) {
      Scalar tmp;
      tmp = source >> 0;
      target[0] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 8;
      target[1] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 16;
      target[2] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 24;
      target[3] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 32;
      target[4] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 40;
      target[5] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 48;
      target[6] = reinterpret_cast<unsigned char&>(tmp);
      tmp = source >> 56;
      target[7] = reinterpret_cast<unsigned char&>(tmp);
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
