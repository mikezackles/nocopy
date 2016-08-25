#ifndef UUID_06CEC703_2131_48E7_BC79_F84A9F0F44BB
#define UUID_06CEC703_2131_48E7_BC79_F84A9F0F44BB

#include <nocopy/fwd/archive.hpp>

#include <nocopy/detail/align_to.hpp>
#include <nocopy/detail/lambda_overload.hpp>
#include <nocopy/detail/narrow_cast.hpp>
#include <nocopy/detail/reference.hpp>
#include <nocopy/detail/traits.hpp>
#include <nocopy/errors.hpp>

#include <nocopy/detail/ignore_warnings_from_dependencies.hpp>
BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#include <span.h>
END_IGNORE_WARNINGS_FROM_DEPENDENCIES

#include <algorithm>
#include <cassert>

namespace nocopy {
  template <typename Offset>
  class archive final {
    using reference = detail::reference<Offset>;

    template <typename T, bool is_single>
    using generic_reference = typename reference::template generic<T, is_single>;

  public:
    template <typename T>
    using single_reference = typename reference::template single<T>;

    template <typename T>
    using range_reference = typename reference::template range<T>;

    archive(unsigned char* buffer, std::size_t capacity) noexcept
      : buffer_{buffer}, capacity_{capacity}, cursor_{0}
    {}

    template <typename T, typename ...Callbacks>
    auto alloc(Callbacks... callbacks) {
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
    auto alloc_range(Offset count, Callbacks... callbacks) noexcept {
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
    auto add(T const& t, Callbacks... callbacks) noexcept {
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
    auto add(gsl::span<T> data, Callbacks... callbacks) noexcept {
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
    auto add(char const* str, std::size_t len, Callbacks... callbacks) {
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
    auto add(std::string const& str, Callbacks... callbacks) {
      return this->add(str.c_str(), str.length(), callbacks...);
    }

    template <typename T, bool Unused>
    decltype(auto) deref(generic_reference<T, Unused> const& ref) const noexcept {
      auto offset = static_cast<Offset>(ref);
      return ref.deref(reinterpret_cast<T const*>(&buffer_[offset]));
    }
    template <typename T, bool Unused>
    decltype(auto) deref(generic_reference<T, Unused>& ref) noexcept {
      auto offset = static_cast<Offset>(ref);
      return ref.deref(reinterpret_cast<T*>(&buffer_[offset]));
    }

    template <typename Callback>
    auto get_data(Callback&& callback) const {
      return callback(gsl::span<unsigned char>{buffer_, cursor_});
    }

  private:
    template <typename T, typename Success, typename Error>
    auto alloc_helper(std::size_t count, Success&& success_callback, Error&& error_callback) noexcept {
      detail::assert_valid_type<T>();
      cursor_ = detail::align_to(cursor_, detail::alignment_for<T>());
      auto size = sizeof(T) * count;
      if (capacity_ - cursor_ < size) {
        return error_callback(make_error_code(error::out_of_space));
      } else {
        auto addr = cursor_;
        cursor_ += size;
        return success_callback(addr, count);
      }
    }

    unsigned char* buffer_;
    Offset capacity_;
    Offset cursor_;
  };
}

#endif
