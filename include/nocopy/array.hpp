#ifndef UUID_6D72B1CC_C284_42C0_85B5_5BF823937719
#define UUID_6D72B1CC_C284_42C0_85B5_5BF823937719

namespace nocopy {
  template <typename T, std::size_t Count>
  struct array {
    static_assert(Count > 0, "array field must have count greater than 0");
  };
}

#endif
