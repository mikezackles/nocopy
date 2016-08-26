#ifndef UUID_8C407DD4_C61B_497E_B9A4_26FB85D27BFC
#define UUID_8C407DD4_C61B_497E_B9A4_26FB85D27BFC

#include <nocopy/fwd/archive.hpp>
#include <nocopy/fwd/heap.hpp>

#include <nocopy/structpack.hpp>
#include <nocopy/field.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <span.h>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

namespace nocopy { namespace detail {
  template <typename Offset>
  class reference {
    NOCOPY_FIELD(offset_field, Offset);
    NOCOPY_FIELD(count_field, Offset);

    template <typename T, bool is_single>
    struct reference_impl;

    template <typename T>
    struct reference_impl<T, true> {
      using base_type = structpack<offset_field_t>;
      explicit operator Offset() const { return data[offset_field]; }
      constexpr T const& deref(T const* t) const { return *t; }
      constexpr T& deref(T* t) { return *t; }

      template <typename Field>
      auto const& operator[](Field f) const { return data[f]; }

      template <typename Field>
      auto& operator[](Field f) { return data[f]; }

      base_type data;
    };

    template <typename T>
    struct reference_impl<T, false> {
      using base_type = structpack<offset_field_t, count_field_t>;
      explicit operator Offset() const { return data[offset_field]; }
      auto deref(T const* t) const {
        using index_type = typename gsl::span<T const>::index_type;
        return gsl::span<T const>{t, static_cast<index_type>(data[count_field])};
      }
      auto deref(T* t) {
        using index_type = typename gsl::span<T const>::index_type;
        return gsl::span<T>{t, static_cast<index_type>(data[count_field])};
      }

      template <typename Field>
      auto const& operator[](Field f) const { return data[f]; }

      template <typename Field>
      auto& operator[](Field f) { return data[f]; }

      base_type data;
    };

    // These types should not be instantiated manually! Use create_single and create_range instead.
    template <typename T, bool is_single>
    using generic = reference_impl<T, is_single>;

    template <typename T>
    using single = generic<T, true>;

    template <typename T>
    using range = generic<T, false>;

    template <typename T>
    static single<T> create_single(Offset offset) {
      single<T> ref{};
      ref[offset_field] = offset;
      return ref;
    }

    template <typename T>
    static range<T> create_range(Offset offset, Offset count) {
      range<T> ref{};
      ref[offset_field] = offset;
      ref[count_field] = count;
      return ref;
    }

    template <typename, typename, bool>
    friend class ::nocopy::detail::heap;
    template <typename>
    friend class ::nocopy::archive;
  };
}}

#endif
