#ifndef UUID_45019CBC_4F2E_47AC_9952_EEAB055B6EEB
#define UUID_45019CBC_4F2E_47AC_9952_EEAB055B6EEB

#include <nocopy/detail/align_to.hpp>
#include <nocopy/detail/lambda_overload.hpp>
#include <nocopy/detail/narrow_cast.hpp>
#include <nocopy/detail/traits.hpp>
#include <nocopy/box.hpp>
#include <nocopy/datapack.hpp>
#include <nocopy/field.hpp>
#include <nocopy/errors.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <span.h>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

#include <cassert>
#include <tuple>
#include <type_traits>

namespace nocopy { namespace detail {
  template <typename Offset, typename AlignmentType, bool AssumeSameSizedByte>
  class heap final {
    static constexpr auto alignment = sizeof(AlignmentType);
    static constexpr std::size_t byte_multiplier = AssumeSameSizedByte ? 1 : CHAR_BIT;

    static_assert(std::is_unsigned<Offset>::value, "Offset type must be signed");
    static_assert(
      std::is_unsigned<AlignmentType>::value
    , "Use an unsigned type (e.g., uint32_t or uint64_t) as the alignment type"
    );
    static_assert(sizeof(Offset) <= sizeof(std::size_t), "");
    static_assert(sizeof(Offset) <= alignment, "Offset must be smaller than alignment");

    struct block_header {
      NOCOPY_FIELD(size, Offset);
      NOCOPY_FIELD(prev, Offset);
      NOCOPY_FIELD(next_free, Offset);
      NOCOPY_FIELD(prev_free, Offset);
      using type = datapack<size, prev, next_free, prev_free>;
    };
    using block_header_t = typename block_header::type;
    static constexpr Offset block_header_size = detail::narrow_cast<Offset>(
      byte_multiplier * detail::align_to(sizeof(block_header_t), alignment)
    );

    static constexpr Offset list_head_offset = 0;
    static constexpr Offset first_block_offset = block_header_size;

    static constexpr Offset initial_bookkeeping = 3 * block_header_size;

    template <typename T, bool single_allocation>
    struct range {};

    template <typename T>
    struct range<T, false> {
      range(Offset count) : count_{count} {
        assert(count_ > 0);
      }
      auto unpack(T const* t) const {
        using index_type = typename gsl::span<T const>::index_type;
        return gsl::span<T const>{t, static_cast<index_type>(count_)};
      }
      auto unpack(T* t) {
        using index_type = typename gsl::span<T const>::index_type;
        return gsl::span<T>{t, static_cast<index_type>(count_)};
      }
    private:
      Offset count_;
    };

    template <typename T>
    struct range<T, true> {
      range() {}
      constexpr T const& unpack(T const* t) const { return *t; }
      constexpr T& unpack(T* t) { return *t; }
    };

  public:
    using offset_t = Offset; // for client code

    template <typename T, Offset Count = 0>
    struct reference : range<T, Count> {
      explicit operator Offset() const { return offset_; }
    private:
      friend class heap;
      reference(Offset offset, Offset extent = Count) : range<T, Count>(extent),  offset_{offset} {}
      Offset offset_;
    };

    template <typename ...Args>
    static auto create(Args... args) noexcept {
      return create_helper(true, args...);
    }

    template <typename ...Args>
    static auto load(Args... args) noexcept {
      return create_helper(false, args...);
    }

    template <typename T, Offset Count>
    decltype(auto) deref(reference<T, Count> const ref) const noexcept {
      auto offset = static_cast<Offset>(ref);
      return ref.unpack(reinterpret_cast<T const*>(&buffer_[offset]));
    }
    template <typename T, Offset Count>
    decltype(auto) deref(reference<T, Count> ref) noexcept {
      auto offset = static_cast<Offset>(ref);
      return ref.unpack(reinterpret_cast<T*>(&buffer_[offset]));
    }

    template <typename T, Offset Count, typename ...Callbacks>
    auto malloc(Callbacks... callbacks) {
      detail::assert_valid_type<T>();
      auto callback = detail::make_overload(std::move(callbacks)...);
      return malloc_helper(
        sizeof(T) * Count
      , [&callback](Offset offset) {
          return callback(reference<T, true>{offset});
        }
      , [&callback](std::error_code e) { return callback(e); }
      );
    }

    template <typename T, typename ...Callbacks>
    auto malloc(Offset count, Callbacks... callbacks) {
      detail::assert_valid_type<T>();
      auto callback = detail::make_overload(std::move(callbacks)...);
      return malloc_helper(
        sizeof(T) * count
      , [&callback, count](Offset offset) {
          return callback(reference<T, false>{offset, count});
        }
      , [&callback](std::error_code e) { return callback(e); }
      );
    }

    template <typename T, Offset Count>
    void free(reference<T, Count> ref) noexcept {
      auto offset = static_cast<Offset>(ref);
      assert(0 < offset && offset < size_);
      auto& block = get_header(offset - block_header_size);
      auto& merged = merge_free_blocks(block); // marks block as free
      add_to_free_list(merged);
    }

    // This is to facilitate testing
    template <typename Callback>
    void each_block(Callback&& callback) const {
      Offset offset = first_block_offset;
      while(offset < sentinel_offset()) {
        auto& block = get_header(offset);
        Offset size; bool is_free;
        std::tie(size, is_free) = get_block_size(block);
        callback(size, is_free, offset + block_header_size);
        offset += block_header_size + size;
      }
      assert(offset == sentinel_offset());
    }

    // This is to facilitate testing
    template <typename Callback>
    void each_free_block(Callback&& callback) const {
      each_free([this, &callback](auto& block) {
        Offset size; bool is_free;
        std::tie(size, is_free) = get_block_size(block);
        assert(is_free);
        auto offset = get_offset(block) + block_header_size;
        callback(size, offset);
        return false;
      });
    }

  private:
    template <typename ...Callbacks>
    auto malloc_helper(std::size_t requested_size, Callbacks... callbacks) {
      auto callback = detail::make_overload(std::move(callbacks)...);
      Offset target_size = detail::narrow_cast<Offset>(
        byte_multiplier * detail::align_to(requested_size, alignment)
      );
      assert(target_size < size_);
      Offset offset;
      auto success = each_free([this, &offset, target_size](auto& block) {
        if (trim(block, target_size)) {
          remove_from_free_list(block);
          mark_as_allocated(block);
          auto result_offset = get_offset(block) + block_header_size;
          assert(
            first_block_offset + block_header_size <= result_offset
            && result_offset < size_ - target_size
          );
          offset = result_offset;
          return true;
        } else {
          return false;
        }
      });
      if (success) {
        return callback(offset);
      } else {
        return callback(make_error_code(error::out_of_space));
      }
    }

    template <typename Self, typename Callback>
    static bool each_free(Self&& self, Callback&& callback) {
      Offset offset = self.get_header(list_head_offset)
        .template get<typename block_header::next_free>();
      while (offset != list_head_offset) {
        assert(0 < offset && offset < self.size_);
        auto& block = self.get_header(offset);
        if (callback(block)) {
          return true;
        }
        offset = block.template get<typename block_header::next_free>();
      }
      return false;
    }
    template <typename Callback>
    bool each_free(Callback&& callback) && {
      return each_free(std::move(*this), std::forward<Callback>(callback));
    }
    template <typename Callback>
    bool each_free(Callback&& callback) & {
      return each_free(*this, std::forward<Callback>(callback));
    }
    template <typename Callback>
    bool each_free(Callback&& callback) const&& {
      return each_free(std::move(*this), std::forward<Callback>(callback));
    }
    template <typename Callback>
    bool each_free(Callback&& callback) const& {
      return each_free(*this, std::forward<Callback>(callback));
    }

    static bool is_aligned(unsigned char* buffer) {
      return (reinterpret_cast<std::uintptr_t>(buffer) & (alignment - 1)) == 0;
    }

    static bool is_heap_big_enough(Offset size) {
      return initial_bookkeeping < size;
    }

    static bool is_heap_too_big(Offset size) {
      return std::numeric_limits<Offset>::max() / byte_multiplier < size;
    }

    void init() {
      new (&buffer_[list_head_offset]) block_header_t{};
      new (&buffer_[first_block_offset]) block_header_t{};
      new (&buffer_[sentinel_offset()]) block_header_t{};
      list_head().template get<typename block_header::prev_free>() = list_head_offset;
      list_head().template get<typename block_header::next_free>() = list_head_offset;
      first_block().template get<typename block_header::size>() = size_ - initial_bookkeeping;
      mark_as_allocated(list_head());
      mark_as_free(first_block());
      mark_as_allocated(sentinel());
      add_to_free_list(first_block());
    }

    block_header_t const& list_head() const {
      return get_header(list_head_offset);
    }
    block_header_t& list_head() {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).list_head());
    }

    block_header_t const& first_block() const {
      return get_header(first_block_offset);
    }
    block_header_t& first_block() {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).first_block());
    }

    // LIFO
    void add_to_free_list(block_header_t& block) {
      auto& head = list_head();
      auto& next = next_free(head);
      block.template get<typename block_header::next_free>()
        = head.template get<typename block_header::next_free>();
      block.template get<typename block_header::prev_free>()
        = next.template get<typename block_header::prev_free>();
      auto block_offset = get_offset(block);
      head.template get<typename block_header::next_free>() = block_offset;
      next.template get<typename block_header::prev_free>() = block_offset;
    }

    void remove_from_free_list(block_header_t& block) {
      auto& prev = prev_free(block);
      auto& next = next_free(block);
      prev.template get<typename block_header::next_free>()
        = block.template get<typename block_header::next_free>();
      next.template get<typename block_header::prev_free>()
        = block.template get<typename block_header::prev_free>();
    }

    bool trim(block_header_t& block, Offset target_size) {
      Offset block_size; bool is_free;
      std::tie(block_size, is_free) = get_block_size(block);
      assert(is_free);
      if (target_size > block_size) return false;
      auto remaining_size = block_size - target_size;
      if (remaining_size >= block_header_size) {
        block.template get<typename block_header::size>() = target_size;
        auto& remainder = next_adjacent(block);
        remainder.template get<typename block_header::size>() = remaining_size - block_header_size;
        remainder.template get<typename block_header::prev>() = get_offset(block);
        auto& next = next_adjacent(remainder);
        next.template get<typename block_header::prev>() = get_offset(remainder);
        mark_as_free(remainder);
        add_to_free_list(remainder);
      }
      return true;
    }

    block_header_t& merge_free_blocks(block_header_t& block) {
      auto& prev = prev_adjacent(block);
      auto& next = next_adjacent(block);

      Offset curr_size, prev_size, next_size;
      bool curr_is_free, prev_is_free, next_is_free;
      std::tie(curr_size, curr_is_free) = get_block_size(block);
      std::tie(prev_size, prev_is_free) = get_block_size(prev);
      std::tie(next_size, next_is_free) = get_block_size(next);

      if (prev_is_free && next_is_free) {
        remove_from_free_list(prev);
        remove_from_free_list(next);
        prev.template get<typename block_header::size>()
          = prev_size + block_header_size + curr_size + block_header_size + next_size;
        mark_as_free(prev);
        auto& new_next = next_adjacent(prev);
        new_next.template get<typename block_header::prev>() = get_offset(prev);
        return prev;
      } else if (prev_is_free) {
        remove_from_free_list(prev);
        prev.template get<typename block_header::size>()
          = prev_size + block_header_size + curr_size;
        mark_as_free(prev);
        auto& new_next = next_adjacent(prev);
        new_next.template get<typename block_header::prev>() = get_offset(prev);
        return prev;
      } else if (next_is_free) {
        remove_from_free_list(next);
        block.template get<typename block_header::size>()
          = curr_size + block_header_size + next_size;
        mark_as_free(block);
        auto& new_next = next_adjacent(block);
        new_next.template get<typename block_header::prev>() = get_offset(block);
        return block;
      } else {
        mark_as_free(block);
        return block;
      }
    }

    block_header_t const& next_free(block_header_t const& block) const {
      return get_header(block.template get<typename block_header::next_free>());
    }
    block_header_t& next_free(block_header_t const& block) {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).next_free(block));
    }

    block_header_t const& prev_free(block_header_t const& block) const {
      return get_header(block.template get<typename block_header::prev_free>());
    }
    block_header_t& prev_free(block_header_t const& block) {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).prev_free(block));
    }

    block_header_t const& next_adjacent(block_header_t const& block) const {
      Offset size; bool is_free;
      std::tie(size, is_free) = get_block_size(block);
      return get_header(get_offset(block) + block_header_size + size);
    }
    block_header_t& next_adjacent(block_header_t const& block) {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).next_adjacent(block));
    }

    block_header_t const& prev_adjacent(block_header_t const& block) const {
      return get_header(block.template get<typename block_header::prev>());
    }
    block_header_t& prev_adjacent(block_header_t const& block) {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).prev_adjacent(block));
    }

    static std::tuple<Offset, bool> get_block_size(block_header_t const& block) {
      Offset s = block.template get<typename block_header::size>();
      Offset size = s & ~Offset{1};
      bool is_free = (s & Offset{1}) == 1;
      return std::make_tuple(size, is_free);
    }

    static void mark_as_free(block_header_t& block) {
      block.template get<typename block_header::size>() |= Offset{1};
    }

    static void mark_as_allocated(block_header_t& block) {
      block.template get<typename block_header::size>() &= ~Offset{1};
    }

    Offset sentinel_offset() const {
      return size_ - block_header_size;
    }

    block_header_t const& sentinel() const {
      return get_header(sentinel_offset());
    }
    block_header_t& sentinel() {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).sentinel());
    }

    Offset get_offset(block_header_t const& block) const {
      auto ptrdiff = reinterpret_cast<unsigned char const*>(&block) - buffer_;
      return detail::narrow_cast<Offset>(byte_multiplier * static_cast<std::size_t>(ptrdiff));
    }

    block_header_t const& get_header(Offset offset) const {
      return reinterpret_cast<block_header_t&>(buffer_[offset / byte_multiplier]);
    }
    block_header_t& get_header(Offset offset) {
      return const_cast<block_header_t&>(static_cast<heap const&>(*this).get_header(offset));
    }

    // Size is either in bytes or bits depending on AssumeSameSizedByte
    template <typename ...Callbacks>
    static auto create_helper(
      bool do_init, unsigned char* buffer, std::size_t size, Callbacks... callbacks
    ) noexcept {
      auto callback = detail::make_overload(std::move(callbacks)...);
      auto aligned_size = detail::narrow_cast<Offset>(
        detail::align_backward(size / byte_multiplier, alignment)
      );
      if (!is_aligned(buffer)) {
        return callback(make_error_code(error::heap_not_aligned));
      } else if (!is_heap_big_enough(aligned_size) || is_heap_too_big(aligned_size)) {
        return callback(make_error_code(error::bad_heap_size));
      } else {
        heap result{buffer, aligned_size};
        if (do_init) result.init();
        return callback(result);
      };
    }

    heap(unsigned char* buffer, Offset size) : buffer_{buffer}, size_{size} {}

    unsigned char* buffer_;
    Offset size_;
  };
}

#ifdef UINT32_MAX
  using heap32 = detail::heap<uint32_t, uint32_t, true>; // assumes CHAR_BIT == 8 (has larger capacity)
  using pedantic_heap32 = detail::heap<uint32_t, uint32_t, false>; // supports CHAR_BIT != 8 (smaller capacity)
#endif
#ifdef UINT64_MAX
  using heap64 = detail::heap<uint64_t, uint64_t, true>; // assumes CHAR_BIT == 8 (has larger capacity)
  using pedantic_heap64 = detail::heap<uint64_t, uint64_t, false>; // supports CHAR_BIT != 8 (smaller capacity)
#endif
}

#endif
