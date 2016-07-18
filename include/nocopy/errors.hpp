#ifndef UUID_3F2DD145_3BE0_4A6B_A997_2EE430E02B2F
#define UUID_3F2DD145_3BE0_4A6B_A997_2EE430E02B2F

namespace nocopy {
  enum class error {
    heap_not_aligned
  , bad_heap_size
  , out_of_space
  };

  class error_category : public std::error_category
  {
  public:
    char const* name() const noexcept override {
      return "nocopy errors";
    }
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wreturn-type"
    std::string message(int ev) const override {
      auto e = static_cast<error>(ev);
      switch (e) {
      case error::heap_not_aligned:
        return "Heap not aligned";
      case error::bad_heap_size:
        return "Bad heap size";
      case error::out_of_space:
        return "Heap full";
      }
    }
  #pragma GCC diagnostic pop
  };

  inline
  std::error_category const&
  get_error_category() {
    static error_category cat;
    return cat;
  }

  inline
  std::error_code
  make_error_code(error e) {
    return std::error_code(static_cast<int>(e), get_error_category());
  }
}

// Specialize std::is_error_code_enum to indicate that errors is an
// error_code_enum
namespace std {
  template<> struct is_error_code_enum<nocopy::error>
    : public std::true_type
  {};
}

#endif
