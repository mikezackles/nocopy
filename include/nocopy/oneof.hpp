#ifndef UUID_421EF854_6D66_42A2_847B_61D932450F88
#define UUID_421EF854_6D66_42A2_847B_61D932450F88

#include <nocopy/fwd/box.hpp>
#include <nocopy/fwd/oneof.hpp>

#include <nocopy/detail/traits.hpp>
#include <nocopy/detail/lambda_overload.hpp>
#include <nocopy/detail/narrow_cast.hpp>
#include <nocopy/detail/oneof_packer.hpp>

#include <cassert>
#include <utility>
#include <type_traits>

namespace nocopy {
  namespace detail {
    template <std::size_t Index, typename Packed>
    struct dispatcher {
      template <typename Callback>
      static auto dispatch(std::size_t i, Callback callback) {
        if (Index == i) {
          constexpr auto t = Packed::template lookup_trait<Index>();
          return callback(t);
        } else {
          return dispatcher<Index - 1, Packed>::dispatch(i, std::move(callback));
        }
      }
    };

    template <typename Packed>
    struct dispatcher<0, Packed> {
      template <typename Callback>
      static auto dispatch(std::size_t i, Callback callback) {
        if (0 == i) {
          constexpr auto t = Packed::template lookup_trait<0>();
          return callback(t);
        } else {
          assert(false);
        }
      }
    };
  }

  template <typename Tag, typename ...Ts>
  class oneof final {
    static_assert(std::is_unsigned<Tag>::value, "tag type for oneof must be unsigned");

    using packed = detail::oneof_packer<Tag, detail::field_traits<Ts>...>;
  public:
    static constexpr auto alignment() { return packed::alignment(); }
    static constexpr auto size() { return packed::size(); }

    // Note that if we've value or zero-initialized, we default to the first
    // type passed as a template argument
    template <typename ...Lambdas>
    auto visit(Lambdas... lambdas) const {
      return detail::dispatcher<packed::num_types() - 1, packed>::dispatch(
        get_tag()
      , [this, callback = detail::make_overload(std::move(lambdas)...)] (auto t) {
          return callback(typename decltype(t)::original_type{}, this->get_payload<decltype(t)>());
        }
      );
    }

    template <typename T>
    auto& get() {
      static_assert(packed::template is_allowed<detail::field_traits<T>>()
      , "a union cannot contain a type not in the union");
      set_tag(detail::narrow_cast<Tag>(packed::template tag_for_type<detail::field_traits<T>>()));
      return get_payload<detail::field_traits<T>>();
    }

  private:
    Tag get_tag() const {
      return reinterpret_cast<detail::boxer_t<Tag> const&>(buffer[0]);
    }

    void set_tag(Tag tag) {
      reinterpret_cast<detail::boxer_t<Tag>&>(mutable_buffer()[0]) = tag;
    }

    template <typename T>
    auto const& get_payload() const {
      return reinterpret_cast<typename T::return_type const&>(
        buffer[packed::payload_offset()]
      );
    }

    template <typename T>
    auto& get_payload() {
      return const_cast<typename T::return_type&>(
        static_cast<oneof const&>(*this).get_payload<T>()
      );
    }

    using buffer_type = std::array<unsigned char, size()>;
  public:
    oneof& operator=(oneof const& other) {
      mutable_buffer() = other.buffer;
      return *this;
    }

    // This must be public for oneof to be an aggreggate
    alignas(alignment()) buffer_type const buffer;

  private:
    buffer_type& mutable_buffer() {
      return const_cast<buffer_type&>(buffer);
    }
  };

  template <typename ...Ts>
  using oneof8 = oneof<uint8_t, Ts...>;

  template <typename ...Ts>
  using oneof16 = oneof<uint16_t, Ts...>;
}

#endif
