#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include <nocopy/fwd/structpack.hpp>

#include <nocopy/detail/field_packer.hpp>
#include <nocopy/detail/traits.hpp>

namespace nocopy {
  template <typename ...Fields>
  class structpack {
    using fieldpack = detail::field_packer<detail::field_traits<Fields>...>;
    using buffer_type = std::array<unsigned char, fieldpack::packed_size>;
  public:
    static constexpr auto alignment() { return fieldpack::alignment; }

    template <typename Field>
    static constexpr bool has() { return fieldpack::template has<detail::field_traits<Field>>(); }

    template <typename Field>
    auto const& get() const {
      return reinterpret_cast<typename detail::field_traits<Field>::return_type const&>(
        buffer[fieldpack::template get_offset<detail::field_traits<Field>>()]
      );
    }

    template <typename Field>
    auto& get() {
      return const_cast<typename detail::field_traits<Field>::return_type&>(
        static_cast<structpack const&>(*this).template get<Field>()
      );
    }

    structpack(structpack const& other) = default;

    structpack& operator=(structpack const& other) {
      mutable_buffer() = other.buffer;
      return *this;
    }

    // This must be public for structpack to be an aggreggate
    alignas(alignment()) buffer_type const buffer;

  private:
    buffer_type& mutable_buffer() {
      return const_cast<buffer_type&>(buffer);
    }
  };
}

#endif
