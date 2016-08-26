#ifndef UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C
#define UUID_322FB7EB_6C3F_48FE_A851_5A140DE1451C

#include <nocopy/fwd/box.hpp>
#include <nocopy/detail/little_endian.hpp>

#include <type_traits>

namespace nocopy {
  template <typename Scalar>
  class box {
    static_assert(std::is_scalar<Scalar>::value, "boxes are for scalar types");
    using buffer_type = std::array<unsigned char, sizeof(Scalar)>;
  public:
    template <typename T>
    auto operator|=(T scalar) -> box<decltype(Scalar() | scalar)>& {
      Scalar val = *this;
      *this = val | scalar;
      return *this;
    }
    template <typename T>
    auto operator&=(T scalar) -> box<decltype(Scalar() & scalar)>& {
      Scalar val = *this;
      *this = val & scalar;
      return *this;
    }
    box& operator=(Scalar scalar) {
      detail::little_endian<Scalar>::store(scalar, mutable_buffer());
      return *this;
    }
    operator Scalar() const { return detail::little_endian<Scalar>::load(buffer); }
    box& operator=(box const& other) {
      mutable_buffer() = other.buffer;
      return *this;
    }

    // This must be public for box to be an aggreggate
    alignas(sizeof(Scalar)) buffer_type const buffer;

  private:
    buffer_type& mutable_buffer() {
      return const_cast<buffer_type&>(buffer);
    }
  };
}

#endif
