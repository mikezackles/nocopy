#ifndef UUID_06CEC703_2131_48E7_BC79_F84A9F0F44BB
#define UUID_06CEC703_2131_48E7_BC79_F84A9F0F44BB

#include <nocopy/fwd/archive.hpp>

#include <nocopy/detail/align_to.hpp>
#include <nocopy/detail/lambda_overload.hpp>
#include <nocopy/detail/narrow_cast.hpp>
#include <nocopy/detail/reference.hpp>
#include <nocopy/detail/traits.hpp>
#include <nocopy/errors.hpp>
#include <nocopy/field.hpp>

#include <silence_dependencies/ignore_warnings.hpp>
SD_BEGIN_IGNORE_WARNINGS
#include <span.h>
SD_END_IGNORE_WARNINGS

#include <algorithm>
#include <cassert>

namespace nocopy {
  namespace detail {
    template <typename Offset, Offset Capacity>
    class archive {
      NOCOPY_FIELD(buffer, NOCOPY_ARRAY(unsigned char, Capacity));
      NOCOPY_FIELD(cursor, Offset);

      using reference = detail::reference<Offset>;

      template <typename T, bool is_single>
      using generic_reference = typename reference::template generic<T, is_single>;

    public:
      using delegate_type = structpack<buffer_t, cursor_t>;

      archive(archive const&) = delete;

      template <typename T>
      using single_reference = typename reference::template single<T>;

      template <typename T>
      using range_reference = typename reference::template range<T>;

      template <typename T, typename ...Callbacks>
      auto alloc(Callbacks... callbacks) const {
        auto callback = detail::make_overload(std::move(callbacks)...);
        return alloc_helper<T>(
          1
        , [&](Offset offset, Offset) {
            return callback(reference::template create_single<T>(offset));
          }
        , callback
        );
      }

      template <typename T, typename ...Callbacks>
      auto alloc_range(Offset count, Callbacks... callbacks) const {
        auto callback = detail::make_overload(std::move(callbacks)...);
        return alloc_helper<T>(
          count
        , [&](Offset offset, Offset count) {
            return callback(reference::template create_range<T>(offset, count));
          }
        , callback
        );
      }

      template <typename T, typename ...Callbacks>
      auto add(T const& t, Callbacks... callbacks) const {
        auto callback = detail::make_overload(std::move(callbacks)...);
        return alloc<T>(
          [=](auto& ref) {
            this->deref(ref) = t;
            return callback(ref);
          }
        , callback
        );
      }

      template <typename T, typename ...Callbacks>
      auto add(gsl::span<T> data, Callbacks... callbacks) const {
        auto callback = detail::make_overload(std::move(callbacks)...);
        return alloc_range<T>(
          data.length()
        , [=](auto& ref) {
            std::copy(data.cbegin(), data.cend(), this->deref(ref).begin());
            return callback(ref);
          }
        , callback
        );
      }

      template <typename ...Callbacks>
      auto add(char const* str, std::size_t len, Callbacks... callbacks) const {
        using span = gsl::span<char const>;
        static_assert(sizeof(Offset) <= sizeof(typename span::index_type)
        , "offset type is too large");
        assert(str != nullptr);
        auto callback = detail::make_overload(std::move(callbacks)...);
        // narrow_cast is necessary because gsl::span uses a signed index type
        auto in = span{str, detail::narrow_cast<typename span::index_type>(len)};
        return alloc_range<char const>(
          len + 1
        , [=](auto& ref) {
            auto& data = this->deref(ref);
            std::copy(in.cbegin(), in.cend(), data.begin());
            data[len] = '\0';
            return callback(ref);
          }
        , callback
        );
      }

      template <typename ...Callbacks>
      auto add(std::string const& str, Callbacks... callbacks) const {
        return this->add(str.c_str(), str.length(), callbacks...);
      }

      template <typename T, bool Unused>
      decltype(auto) deref(generic_reference<T, Unused> const& ref) const noexcept {
        auto offset = static_cast<Offset>(ref);
        return ref.deref(data[buffer][offset]);
      }

      char const* get_string(range_reference<char const> const& ref) const noexcept {
        return this->deref(ref).data();
      }

      delegate_type data;

    private:
      template <typename T, typename Success, typename Error>
      auto alloc_helper(std::size_t count, Success&& success_callback, Error&& error_callback) noexcept {
        detail::assert_valid_type<T>();
        Offset addr = detail::align_to(data[cursor], detail::alignment_for<T>());
        auto size = sizeof(T) * count;
        if (Capacity - addr < size) {
          return error_callback(make_error_code(error::out_of_space));
        } else {
          data[cursor] = addr + size;
          return success_callback(addr, count);
        }
      }
    };
  }
//#ifdef UINT32_MAX
//  template <uint32_t Capacity>
//  using archive32 = detail::archive_impl<uint32_t, Capacity>;
//#endif
//#ifdef UINT64_MAX
//  template <uint64_t Capacity>
//  using archive64 = detail::archive_impl<uint64_t, Capacity>;
//#endif
}

#endif
