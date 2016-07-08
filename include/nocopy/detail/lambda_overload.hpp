#ifndef UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878
#define UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878

namespace nocopy { namespace detail {
  template <typename ...Lambdas>
  struct lambda_overload : Lambdas... {
    lambda_overload(Lambdas... lambdas) : Lambdas{std::move(lambdas)}... {}
  };
}}

#endif
