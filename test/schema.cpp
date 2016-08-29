#include <catch.hpp>

#include <nocopy.hpp>

struct nested {
  NOCOPY_FIELD(a, uint16_t);
  NOCOPY_FIELD(b, int8_t);
  NOCOPY_FIELD(c, int32_t);
  NOCOPY_FIELD(d, int32_t);
  NOCOPY_FIELD(e, NOCOPY_ONEOF(c_t, d_t));

  template <std::size_t Version>
  using v =
  nocopy::schema<
    Version
  , nocopy::version_range<a_t, 4>
  , nocopy::version_range<b_t, 4, 4>
  , nocopy::version_range<c_t, 4>
  , nocopy::version_range<e_t, 5>
  >;
};

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));
  NOCOPY_VERSIONED_FIELD(nested_field, nested);

  //                     field removed in this version
  //                    field added in this version  |
  template <std::size_t Version> //               |  |
  using v = //                                    |  |
  nocopy::schema< //                              |  |
    Version //                                    v  v
  , nocopy::version_range< delta_t,               0    >
  , nocopy::version_range< first_t,               0, 1 >
  , nocopy::version_range< coords_t,              0    >
  , nocopy::version_range< locations_t,           1    >
  , nocopy::version_range< second_t,              2, 4 >
  , nocopy::version_range< first_t,               3, 5 >
  , nocopy::version_range< nested_field<Version>, 4, 5 >
  >;
};

SCENARIO("archive") {
  using m = measurement;

  GIVEN("a versioned archive") {
    THEN("the correct fields should be present") {
      REQUIRE(m::v<0>::has(m::delta));
      REQUIRE(m::v<0>::has(m::first));
      REQUIRE(!m::v<0>::has(m::second));
      REQUIRE(m::v<0>::has(m::coords));
      REQUIRE(!m::v<0>::has(m::locations));
      REQUIRE(!m::v<0>::has<m::nested_field>());

      REQUIRE(m::v<1>::has(m::delta));
      REQUIRE(m::v<1>::has(m::first));
      REQUIRE(!m::v<1>::has(m::second));
      REQUIRE(m::v<1>::has(m::coords));
      REQUIRE(m::v<1>::has(m::locations));
      REQUIRE(!m::v<1>::has<m::nested_field>());

      REQUIRE(m::v<2>::has(m::delta));
      REQUIRE(!m::v<2>::has(m::first));
      REQUIRE(m::v<2>::has(m::second));
      REQUIRE(m::v<2>::has(m::coords));
      REQUIRE(m::v<2>::has(m::locations));
      REQUIRE(!m::v<2>::has<m::nested_field>());

      REQUIRE(m::v<3>::has(m::delta));
      REQUIRE(m::v<3>::has(m::first));
      REQUIRE(m::v<3>::has(m::second));
      REQUIRE(m::v<3>::has(m::coords));
      REQUIRE(m::v<3>::has(m::locations));
      REQUIRE(!m::v<3>::has<m::nested_field>());

      REQUIRE(m::v<4>::has(m::delta));
      REQUIRE(m::v<4>::has(m::first));
      REQUIRE(m::v<4>::has(m::second));
      REQUIRE(m::v<4>::has(m::coords));
      REQUIRE(m::v<4>::has(m::locations));
      REQUIRE(m::v<4>::has<m::nested_field>());

      REQUIRE(m::v<5>::has(m::delta));
      REQUIRE(m::v<5>::has(m::first));
      REQUIRE(!m::v<5>::has(m::second));
      REQUIRE(m::v<5>::has(m::coords));
      REQUIRE(m::v<5>::has(m::locations));
      REQUIRE(m::v<5>::has<m::nested_field>());

      m::v<5> m_v5{};
      m_v5.get<m::nested_field>()[nested::a] = 4;
    }
  }
}
