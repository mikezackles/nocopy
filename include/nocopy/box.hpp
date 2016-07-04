#ifndef UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C
#define UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C

#include <type_traits>

#if !defined(NOCOPY_LITTLEENDIAN)
  #if defined(__GNUC__) || defined(__clang__)
    #ifdef __BIG_ENDIAN__
      #define NOCOPY_LITTLEENDIAN 0
    #else
      #define NOCOPY_LITTLEENDIAN 1
    #endif // __BIG_ENDIAN__
  #elif defined(_MSC_VER)
    #if defined(_M_PPC)
      #define NOCOPY_LITTLEENDIAN 0
    #else
      #define NOCOPY_LITTLEENDIAN 1
    #endif
  #else
    #error Unable to determine endianness, define NOCOPY_LITTLEENDIAN.
  #endif
#endif // !defined(NOCOPY_LITTLEENDIAN)

namespace nocopy {
  namespace detail {
    template <typename Scalar, typename = hana::when<true>>
    struct swapper {
      static_assert(sizeof(Scalar) > 1 || sizeof(Scalar) <= 8
      , "byte swap not supported for this scalar");
    };

    template <typename Scalar>
    struct swapper<Scalar, hana::when<sizeof(Scalar) == 2>> {
      static Scalar swap(Scalar scalar) {
        #if defined(_MSC_VER)
          #pragma push_macro("__builtin_bswap16")
          #define __builtin_bswap16 _byteswap_ushort
        #endif

        auto r = __builtin_bswap16(*reinterpret_cast<uint16_t *>(&scalar));
        return *reinterpret_cast<Scalar *>(&r);

        #if defined(_MSC_VER)
          #pragma pop_macro("__builtin_bswap16")
        #endif
      }
    };

    template <typename Scalar>
    struct swapper<Scalar, hana::when<sizeof(Scalar) == 4>> {
      static Scalar swap(Scalar scalar) {
        #if defined(_MSC_VER)
          #pragma push_macro("__builtin_bswap32")
          #define __builtin_bswap32 _byteswap_ushort
        #endif

        #ifdef __clang__
          #pragma clang diagnostic push
          #pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
        #endif
        auto r = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&scalar));
        return *reinterpret_cast<Scalar *>(&r);
        #ifdef __clang__
          #pragma clang diagnostic pop
        #endif

        #if defined(_MSC_VER)
          #pragma pop_macro("__builtin_bswap32")
        #endif
      }
    };

    template <typename Scalar>
    struct swapper<Scalar, hana::when<sizeof(Scalar) == 8>> {
      static Scalar swap(Scalar scalar) {
        #if defined(_MSC_VER)
          #pragma push_macro("__builtin_bswap64")
          #define __builtin_bswap64 _byteswap_ushort
        #endif

        auto r = __builtin_bswap64(*reinterpret_cast<uint64_t *>(&scalar));
        return *reinterpret_cast<Scalar *>(&r);

        #if defined(_MSC_VER)
          #pragma pop_macro("__builtin_bswap64")
        #endif
      }
    };
  }

  template <typename Scalar>
  class box {
    static_assert(std::is_scalar<Scalar>::value, "boxes are for scalar types");
  public:
    explicit box(Scalar scalar) : scalar_{byte_swap_if_big_endian(scalar)} {}
    box& operator=(Scalar scalar) {
      scalar_ = byte_swap_if_big_endian(scalar);
      return *this;
    }
    operator Scalar() const { return byte_swap_if_big_endian(scalar_); }
  private:
    static Scalar byte_swap_if_big_endian(Scalar scalar) {
        //#if NOCOPY_LITTLEENDIAN
        //  return scalar;
        //#else
        return detail::swapper<Scalar>::swap(scalar);
        //#endif
    }
    Scalar scalar_;
  };
}

#endif
