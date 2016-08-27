#include <catch.hpp>

#include <nocopy.hpp>

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));

  //           field removed in this version
  //          field added in this version  |
  template <std::size_t Version> //     |  |
  using v = //                       |  |
  nocopy::schema< //                    |  |
    nocopy::structpack //               |  |
  , Version //                          v  v
  , nocopy::version_range< delta_t,     0    >
  , nocopy::version_range< first_t,     0, 1 >
  , nocopy::version_range< coords_t,    0    >
  , nocopy::version_range< locations_t, 1    >
  , nocopy::version_range< second_t,    2, 4 >
  , nocopy::version_range< first_t,     3, 5 >
  >;
};

SCENARIO("archive") {
  GIVEN("a versioned archive") {
    THEN("the correct fields should be present") {
      REQUIRE(measurement::v<0>::has(measurement::delta));
      REQUIRE(measurement::v<0>::has(measurement::first));
      REQUIRE(!measurement::v<0>::has(measurement::second));
      REQUIRE(measurement::v<0>::has(measurement::coords));
      REQUIRE(!measurement::v<0>::has(measurement::locations));

      REQUIRE(measurement::v<1>::has(measurement::delta));
      REQUIRE(measurement::v<1>::has(measurement::first));
      REQUIRE(!measurement::v<1>::has(measurement::second));
      REQUIRE(measurement::v<1>::has(measurement::coords));
      REQUIRE(measurement::v<1>::has(measurement::locations));

      REQUIRE(measurement::v<2>::has(measurement::delta));
      REQUIRE(!measurement::v<2>::has(measurement::first));
      REQUIRE(measurement::v<2>::has(measurement::second));
      REQUIRE(measurement::v<2>::has(measurement::coords));
      REQUIRE(measurement::v<2>::has(measurement::locations));

      REQUIRE(measurement::v<3>::has(measurement::delta));
      REQUIRE(measurement::v<3>::has(measurement::first));
      REQUIRE(measurement::v<3>::has(measurement::second));
      REQUIRE(measurement::v<3>::has(measurement::coords));
      REQUIRE(measurement::v<3>::has(measurement::locations));

      REQUIRE(measurement::v<4>::has(measurement::delta));
      REQUIRE(measurement::v<4>::has(measurement::first));
      REQUIRE(measurement::v<4>::has(measurement::second));
      REQUIRE(measurement::v<4>::has(measurement::coords));
      REQUIRE(measurement::v<4>::has(measurement::locations));

      REQUIRE(measurement::v<5>::has(measurement::delta));
      REQUIRE(measurement::v<5>::has(measurement::first));
      REQUIRE(!measurement::v<5>::has(measurement::second));
      REQUIRE(measurement::v<5>::has(measurement::coords));
      REQUIRE(measurement::v<5>::has(measurement::locations));
    }
  }
}
