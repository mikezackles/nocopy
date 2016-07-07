#ifndef UUID_421EF854_6D66_42A2_847B_61D932450F88
#define UUID_421EF854_6D66_42A2_847B_61D932450F88

#include <nocopy/fwd/box.hpp>
#include <nocopy/fwd/oneof.hpp>

#include <nocopy/detail/align_to.hpp>
#include <nocopy/detail/oneof_packer.hpp>

#include <type_traits>

namespace nocopy {
  template <typename Tag, typename ...Ts>
  class oneof final {
    static_assert(std::is_unsigned<Tag>::value, "tag type for oneof must be unsigned");

    using packed = detail::oneof_packer<Tag, Ts...>;
  public:
    static constexpr auto alignment() { return packed::alignment(); }
    static constexpr auto size() { return packed::size(); }

    // Note that if we've value or zero-initialized, we default to the first
    // type passed as a template argument
    template <typename Visitor>
    void visit(Visitor&& visitor) const {
      visitor(get_payload(lookup_type(get_tag())));
    }

    template <typename T>
    auto& get() {
      static_assert(packed::template is_allowed<T>()
      , "a union cannot contain a type not in the union");
      set_tag(tag_for_type(hana::type_c<T>));
    }
  private:
    Tag get_tag() const {
      return reinterpret_cast<box<Tag> const&>(&buffer_[0]);
    }

    void set_tag(Tag tag) {
      reinterpret_cast<box<Tag>&>(&buffer_[0]) = tag;
    }

    template <typename T>
    auto const& get_payload(T) const {
      reinterpret_cast<typename T::type::return_type const&>(
        buffer_[packed::payload_offset()]
      );
    }

    template <typename T>
    auto& get_payload(T t) {
      return const_cast<typename T::type::return_type&>(
        static_cast<oneof const&>(*this).get_payload(t)
      );
    }

    alignas(alignment()) std::array<unsigned char, size()> buffer_;
  };
}

#endif
