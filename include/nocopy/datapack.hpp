#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include <nocopy/fwd/datapack.hpp>

#include <nocopy/detail/field_packer.hpp>
#include <nocopy/detail/traits.hpp>

namespace nocopy {
  template <typename ...Fields>
  class datapack final {
    using fieldpack = detail::field_packer<detail::field_traits<Fields>...>;
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
        static_cast<datapack const&>(*this).template get<Field>()
      );
    }

    // I'd make this private, but then this wouldn't technically be an aggregate
    alignas(alignment()) std::array<unsigned char, fieldpack::packed_size> buffer;
  };
}

#endif
