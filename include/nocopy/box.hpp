#ifndef UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C
#define UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C

#include <nocopy/fwd/box.hpp>
#include <nocopy/detail/little_endian.hpp>

#include <type_traits>

namespace nocopy {
  template <typename Scalar>
  class box {
    static_assert(std::is_scalar<Scalar>::value, "boxes are for scalar types");
  public:
    box& operator=(Scalar scalar) {
      detail::little_endian<Scalar>::store(scalar, buffer_);
      return *this;
    }
    operator Scalar() const { return detail::little_endian<Scalar>::load(buffer_); }
  private:
    alignas(sizeof(Scalar)) std::array<unsigned char, sizeof(Scalar)> buffer_;
  };
}

#endif
