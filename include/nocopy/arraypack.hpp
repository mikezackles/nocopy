#ifndef UUID_E7D186DD_98F4_410C_A45A_9C39E7680281
#define UUID_E7D186DD_98F4_410C_A45A_9C39E7680281

#include "detail/field_accessors.hpp"
#include "detail/traits.hpp"

#include <array>

namespace nocopy {
  template <typename T, std::size_t Size>
  class arraypack {
  public:
    using size_type = std::size_t;

    decltype(auto) get(std::size_t index) {
      assert(reinterpret_cast<std::uintptr_t>(this) % detail::find_alignment<T>::result == 0);
      assert(index < Size);

      return detail::field_getter<T>::get(buffer_[index]);
    }

    decltype(auto) get(std::size_t index) const {
      assert(reinterpret_cast<std::uintptr_t>(this) % detail::find_alignment<T>::result == 0);
      assert(index < Size);

      return detail::field_getter<T const>::get(buffer_[index]);
    }

    void set(std::size_t index, T val) {
      assert(reinterpret_cast<std::uintptr_t>(this) % detail::find_alignment<T>::result == 0);
      assert(index < Size);

      return detail::field_setter<T>::set(buffer_[index], val);
    }

    size_type size() { return buffer_.size(); }

  private:
    std::array<T, Size> buffer_;
  };
}

#endif
