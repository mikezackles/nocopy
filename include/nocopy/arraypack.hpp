#ifndef UUID_E7D186DD_98F4_410C_A45A_9C39E7680281
#define UUID_E7D186DD_98F4_410C_A45A_9C39E7680281

#include "detail/field_accessors.hpp"

namespace nocopy {
  template <typename T, std::size_t Size>
  class arraypack {
  public:
    decltype(auto) get(std::size_t index) {
      return detail::field_getter<T>::get(buffer_[index]);
    }

    decltype(auto) get(std::size_t index) const {
      return detail::field_getter<T const>::get(buffer_[index]);
    }

    void set(std::size_t index, T val) {
      return detail::field_setter<T>::set(buffer_[index], val);
    }

  private:
    std::array<T, Size> buffer_;
  };
}

#endif
