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
      template <typename StructPack, typename ...Field, typename ...Args>
      static void construct(gsl::span<StructPack> range, member_args<Field, Args>... inits) {
        for (auto& s : range) {
          StructPack::construct(s, inits...);
        }
      }
      template <typename StructPack>
      static void destruct(gsl::span<StructPack> range) {
        for (auto& s : range) {
          StructPack::destruct(s);
        }
      }
    };
  }

  template <typename ...Fields>
  class structpack {
    using fieldpack = detail::field_packer<detail::field_traits<Fields>...>;
    using buffer_type = std::array<unsigned char, fieldpack::packed_size>;

    template <typename Field, typename ...Args, std::size_t ...Indices>
    static void construct_helper_indexed(
      structpack& self
    , member_args<Field, std::tuple<Args...>> args
    , std::index_sequence<Indices...>
    ) {
      auto wrapped = self[Field{}];
      decltype(wrapped)::construct(wrapped, std::forward<Args>(std::get<Indices>(args))...);
    }

    template <typename Field, typename ...Args>
    static void construct_helper(
      structpack& self
    , member_args<Field, std::tuple<Args...>> args
    ) {
      construct_helper_indexed(self, args, std::make_index_sequence<sizeof...(Args)>{});
    }

  public:
    static constexpr auto alignment() { return fieldpack::alignment; }

    template <typename Field>
    static constexpr bool has(Field) { return fieldpack::template has<detail::field_traits<Field>>(); }

    template <typename ...Field, typename ...Args>
    static void construct(structpack& self, member_args<Field, Args>... inits) {
      fieldpack::template assert_all_wrapped_fields_present<Field...>();
      using expand_type = int[];
      expand_type{(construct_helper(self, inits), 0)...};
    }

    static void destruct(structpack& self) {
      fieldpack::each_wrapped([&](auto f) {
        auto wrapped = self[f];
        decltype(wrapped)::destruct(wrapped);
      });
    }

    template <typename ...Field, typename ...Args>
    static void construct_range(gsl::span<structpack> range, member_args<Field, Args>... inits) {
      detail::range_helper<fieldpack::has_custom_fields()>::construct(range, inits...);
    }

    static void destruct_range(gsl::span<structpack> range) {
      detail::range_helper<fieldpack::has_custom_fields()>::destruct(range);
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
