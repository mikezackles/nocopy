#ifndef UUID_A6E51180_138B_4B44_9FC7_C8BDD9F96A93
#define UUID_A6E51180_138B_4B44_9FC7_C8BDD9F96A93

namespace nocopy { namespace detail {
  template <typename T>
  struct check_exists { using type = void; };

  template <typename Field, typename = void>
  struct is_delegate : std::false_type {};
  template <typename Field>
  struct is_delegate<
    Field, typename check_exists<typename Field::field_type::delegate_type>::type
  > : std::true_type {};

  template <typename T, typename = void>
  struct find_delegate { using type = T; };
  template <typename T>
  struct find_delegate<
    T, typename check_exists<typename T::delegate_type>::type
  > {
    static_assert(
      sizeof(T) == sizeof(typename T::delegate_type)
    , "delegate type must not add member data"
    );
    using type = typename T::delegate_type;
  };
}}

#endif
