#ifndef UUID_421EF854_6D66_42A2_847B_61D932450F88
#define UUID_421EF854_6D66_42A2_847B_61D932450F88

#include <nocopy/detail/align_to.hpp>

namespace nocopy {
  template <typename ...Ts>
  class oneof final {
    using tag_type = uint16_t;

    static constexpr allowed_types = hana::make_tuple(hana::type_c<detail::field_traits<Ts>>...);
    static constexpr auto lookup_type(tag_type offset) {
      return hana::get(allowed_types, offset);
    }

    struct alignment_ordering {
      template <typename X, typename Y>
      constexpr auto operator()(X, Y) const {
        bool result = X::type::alignment < Y::type::alignment;
        return hana::bool_c<result>;
      }
    };
    static constexpr auto payload_alignment = hana::maximum(allowed_types, alignment_ordering{});

    static constexpr auto payload_offset = align_to(sizeof(tag_type), payload_alignment());

  public:
    static constexpr auto alignment() {
      return std::max(payload_alignment, sizeof(tag_type));
    };
    static constexpr auto size() {
      return align_to(payload_offset + payload_alignment, alignment());
    }

    // Note that if we've value or zero-initialized, we default to the first
    // type passed as a template argument
    template <typename Visitor>
    void visit(Visitor&& visitor) const {
      visitor(get_payload(lookup_type(get_tag())));
    }

    template <typename T>
    auto& get() {
      static_assert(
        hana::contains(allowed_types, hana::type_c<T>)
      , "a union cannot contain a type not in the union");
      set_tag(tag_for_type(hana::type_c<T>));
    }
  private:
    tag_type get_tag() const {
      return reinterpret_cast<box<tag_type> const&>(&buffer_[0]);
    }

    void set_tag(tag_type tag) {
      reinterpret_cast<box<tag_type>&>(&buffer_[0]) = tag;
    }

    template <typename T>
    auto const& get_payload(T) const {
      reinterpret_cast<typename T::type::return_type const&>(
        buffer_[payload_offset]
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
