#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include "detail/field_packer.hpp"
#include "detail/traits.hpp"
#include "static_asserts.hpp"

namespace nocopy {
  namespace hana = boost::hana;

  template <typename ...Fields>
  class datapack final {
    using fieldpack = detail::field_packer<Fields...>;
  public:
    static constexpr auto alignment() { return fieldpack::alignment; }

    template <typename Field>
    static constexpr bool has() { return fieldpack::template has<Field>(); }

    template <typename Field>
    auto const& get() const {
      return reinterpret_cast<typename detail::field_traits<Field>::return_type const&>(
        buffer_[fieldpack::template get_offset<Field>()]
      );
    }

    template <typename Field>
    auto& get() {
      return const_cast<typename detail::field_traits<Field>::return_type&>(
        static_cast<datapack const&>(*this).template get<Field>()
      );
    }

  private:
    alignas(alignment()) std::array<unsigned char, fieldpack::packed_size> buffer_;
  };
}

#endif
