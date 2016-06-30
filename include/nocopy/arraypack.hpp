#ifndef UUID_E7D186DD_98F4_410C_A45A_9C39E7680281
#define UUID_E7D186DD_98F4_410C_A45A_9C39E7680281

#include "endianness.hpp"

namespace nocopy {
  template <typename T, std::size_t Size>
  class arraypack {
  public:
    T get(std::size_t index) const { return byte_swap_if_big_endian(buffer_[index]); }
    void set(std::size_t index, T val) { buffer_[index] = byte_swap_if_big_endian(val); }
  private:
    std::array<T, Size> buffer_;
  };
}

#endif
