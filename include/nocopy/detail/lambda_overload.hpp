#ifndef UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878
#define UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878

namespace nocopy { namespace detail {
  template <typename Lambda, typename ...Lambdas>
  struct lambda_overload : Lambda, lambda_overload<Lambdas...> {
    using Lambda::operator();
    using lambda_overload<Lambdas...>::operator();
    lambda_overload(Lambda lambda, Lambdas... lambdas)
      : Lambda{std::move(lambda)}, lambda_overload<Lambdas...>{std::move(lambdas)...} {}
  };

  template <typename Lambda>
  struct lambda_overload<Lambda> : Lambda {
    using Lambda::operator();
    lambda_overload(Lambda lambda) : Lambda{std::move(lambda)} {}
  };

  template <typename ...Lambdas>
  auto make_overload(Lambdas... lambdas) {
    return lambda_overload<Lambdas...>{std::move(lambdas)...};
  }
}}

#endif
