#ifndef UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E
#define UUID_DF22E38D_D570_4E32_A815_00BD9BAB708E

#include <nocopy/fwd/structpack.hpp>

#include <nocopy/detail/field_packer.hpp>
#include <nocopy/detail/traits.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <span.h>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

#include <tuple>
#include <utility>

namespace nocopy {
  template <typename Field, typename Args>
  struct member_args {
    Args args;
  };

  template <typename Field, typename ...Args>
  auto member_init(Field, Args&&... args) {
    return member_args<Field, std::tuple<Args...>>{
      std::forward_as_tuple(std::forward<Args>(args)...)
    };
  }

  namespace detail {
    template <bool HasCustomFields>
    struct range_helper {
      template <typename StructPack>
      static void construct(gsl::span<StructPack>) {}
      template <typename StructPack>
      static void destruct(gsl::span<StructPack>) {}
    };
    template <>
    struct range_helper<true> {
      template <typename Allocator, typename StructPack, typename ...Field, typename ...Args>
      static void construct(
        Allocator& allocator, gsl::span<StructPack> range, member_args<Field, Args>... inits
      ) {
        for (auto& s : range) {
          StructPack::construct(allocator, s, inits...);
        }
      }
      template <typename Allocator, typename StructPack>
      static void destruct(Allocator& allocator, gsl::span<StructPack> range) {
        for (auto& s : range) {
          StructPack::destruct(allocator, s);
        }
      }
    };
  }

  template <typename ...Fields>
  class structpack {
    using fieldpack = detail::field_packer<detail::field_traits<Fields>...>;
    using buffer_type = std::array<unsigned char, fieldpack::packed_size>;

    template <typename Allocator, typename Field, typename ...Args, std::size_t ...Indices>
    static void construct_helper_indexed(
      Allocator& allocator
    , structpack& self
    , member_args<Field, std::tuple<Args...>> args
    , std::index_sequence<Indices...>
    ) {
      auto wrapped = self[Field{}];
      decltype(wrapped)::construct(allocator, wrapped, std::forward<Args>(std::get<Indices>(args))...);
    }

    template <typename Allocator, typename Field, typename ...Args>
    static void construct_helper(
      Allocator& allocator
    , structpack& self
    , member_args<Field, std::tuple<Args...>> args
    ) {
      construct_helper_indexed(allocator, self, args, std::make_index_sequence<sizeof...(Args)>{});
    }

  public:
    static constexpr auto alignment() { return fieldpack::alignment; }

    template <typename Field>
    static constexpr bool has(Field) { return fieldpack::template has<detail::field_traits<Field>>(); }

    template <typename Allocator, typename ...Field, typename ...Args>
    static void construct(Allocator& allocator, structpack& self, member_args<Field, Args>... inits) {
      fieldpack::template assert_all_delegate_fields_present<Field...>();
      using expand_type = int[];
      expand_type{(construct_helper(allocator, self, inits), 0)...};
    }

    template <typename Allocator>
    static void destruct(Allocator& allocator, structpack& self) {
      fieldpack::each_delegate([&](auto f) {
        auto wrapped = self[f];
        decltype(wrapped)::destruct(allocator, wrapped);
      });
    }

    template <typename Allocator, typename ...Field, typename ...Args>
    static void construct_range(
      Allocator& allocator, gsl::span<structpack> range, member_args<Field, Args>... inits
    ) {
      detail::range_helper<fieldpack::has_custom_fields()>::construct(allocator, range, inits...);
    }

    template <typename Allocator>
    static void destruct_range(Allocator& allocator, gsl::span<structpack> range) {
      detail::range_helper<fieldpack::has_custom_fields()>::destruct(allocator, range);
    }

    template <typename Field>
    decltype(auto) operator[](Field) const {
      return detail::field_traits<Field>::wrap(
        buffer[fieldpack::template get_offset<detail::field_traits<Field>>()]
      );
    }

    template <typename Field>
    decltype(auto) operator[](Field) {
      return detail::field_traits<Field>::wrap(
        mutable_buffer()[fieldpack::template get_offset<detail::field_traits<Field>>()]
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
