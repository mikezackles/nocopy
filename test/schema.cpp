#include <catch.hpp>

#include <nocopy.hpp>

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));

  //         field removed in this version
  //        field added in this version  |
  template <std::size_t Version> //   |  |
  using type = //                     |  |
  nocopy::schema_t< //                |  |
    nocopy::datapack //               |  |
  , Version //                        v  v
  , nocopy::version_range< delta,     0    >
  , nocopy::version_range< first,     0, 1 >
  , nocopy::version_range< coords,    0    >
  , nocopy::version_range< locations, 1    >
  , nocopy::version_range< second,    2, 4 >
  , nocopy::version_range< first,     3, 5 >
  >;
};
template <std::size_t Version>
using measurement_t = typename measurement::type<Version>;


SCENARIO("archive") {
  GIVEN("a versioned archive") {
    THEN("the correct fields should be present") {
      REQUIRE(measurement_t<0>::has<measurement::delta>());
      REQUIRE(measurement_t<0>::has<measurement::first>());
      REQUIRE(!measurement_t<0>::has<measurement::second>());
      REQUIRE(measurement_t<0>::has<measurement::coords>());
      REQUIRE(!measurement_t<0>::has<measurement::locations>());

      REQUIRE(measurement_t<1>::has<measurement::delta>());
      REQUIRE(measurement_t<1>::has<measurement::first>());
      REQUIRE(!measurement_t<1>::has<measurement::second>());
      REQUIRE(measurement_t<1>::has<measurement::coords>());
      REQUIRE(measurement_t<1>::has<measurement::locations>());

      REQUIRE(measurement_t<2>::has<measurement::delta>());
      REQUIRE(!measurement_t<2>::has<measurement::first>());
      REQUIRE(measurement_t<2>::has<measurement::second>());
      REQUIRE(measurement_t<2>::has<measurement::coords>());
      REQUIRE(measurement_t<2>::has<measurement::locations>());

      REQUIRE(measurement_t<3>::has<measurement::delta>());
      REQUIRE(measurement_t<3>::has<measurement::first>());
      REQUIRE(measurement_t<3>::has<measurement::second>());
      REQUIRE(measurement_t<3>::has<measurement::coords>());
      REQUIRE(measurement_t<3>::has<measurement::locations>());

      REQUIRE(measurement_t<4>::has<measurement::delta>());
      REQUIRE(measurement_t<4>::has<measurement::first>());
      REQUIRE(measurement_t<4>::has<measurement::second>());
      REQUIRE(measurement_t<4>::has<measurement::coords>());
      REQUIRE(measurement_t<4>::has<measurement::locations>());

      REQUIRE(measurement_t<5>::has<measurement::delta>());
      REQUIRE(measurement_t<5>::has<measurement::first>());
      REQUIRE(!measurement_t<5>::has<measurement::second>());
      REQUIRE(measurement_t<5>::has<measurement::coords>());
      REQUIRE(measurement_t<5>::has<measurement::locations>());
    }
  }
}
