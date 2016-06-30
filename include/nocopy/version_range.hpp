#ifndef UUID_F2774B94_E4AD_4159_9230_0A991086A19E
#define UUID_F2774B94_E4AD_4159_9230_0A991086A19E

namespace nocopy {
  template <typename Field, std::size_t FirstVersion, std::size_t LastVersion>
  struct version_range {
    using field = Field;
    static constexpr auto first_version = FirstVersion;
    static constexpr auto last_version = LastVersion;
  };
}

#endif
