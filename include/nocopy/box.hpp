#ifndef UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C
#define UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C

#include <cstring>
#include <type_traits>

namespace nocopy {
  namespace detail {
    template <typename Scalar, typename = hana::when<true>>
    struct converter {
      static_assert(1 < sizeof(Scalar) && sizeof(Scalar) <= 8
      , "byte swap not supported for this scalar");
    };

    // The goal here is to keep the code simple and standards compliant for now,
    // hoping that compilers will be smart enough to optimize.
    template <typename Scalar>
    struct converter<Scalar, hana::when<sizeof(Scalar) == 2>> {
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
    struct converter<Scalar, hana::when<sizeof(Scalar) == 4 && !std::is_floating_point<Scalar>::value>> {
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
    struct converter<Scalar, hana::when<sizeof(Scalar) == 8 && !std::is_floating_point<Scalar>::value>> {
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

    template <std::size_t Size> struct converter_type;
    template <> struct converter_type<4> { using type = uint32_t; };
    template <> struct converter_type<8> { using type = uint64_t; };
    template <std::size_t Size> using converter_t = typename converter_type<Size>::type;

    template <typename Scalar>
    struct converter<Scalar, hana::when<std::is_floating_point<Scalar>::value>> {
      using byte_array = std::array<unsigned char, 4>;
      using proxy_type = converter_t<sizeof(Scalar)>;
      static Scalar load(byte_array const& source) {
        proxy_type tmp = converter<proxy_type>::load(source);
        Scalar result;
        std::memcpy(&result, &tmp, sizeof(Scalar));
        return result;
      }
      static void store(Scalar source, byte_array& target) {
        proxy_type tmp;
        std::memcpy(&tmp, &source, sizeof(Scalar));
        converter<proxy_type>::store(tmp, target);
      }
    };
  }

  template <typename Scalar>
  class box {
    static_assert(std::is_scalar<Scalar>::value, "boxes are for scalar types");
  public:
    box& operator=(Scalar scalar) {
      detail::converter<Scalar>::store(scalar, buffer_);
      return *this;
    }
    operator Scalar() const { return detail::converter<Scalar>::load(buffer_); }
  private:
    alignas(sizeof(Scalar)) std::array<unsigned char, sizeof(Scalar)> buffer_;
  };
}

#endif
