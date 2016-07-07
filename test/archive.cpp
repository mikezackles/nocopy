#include <catch.hpp>

#include <nocopy.hpp>

namespace measurement_fields {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));
}

//                             field removed in this version
//                            field added in this version  |
template <std::size_t Version> //                       |  |
using measurement = //                                  |  |
nocopy::archive_t< //                                   |  |
  Version //                                            v  v
, nocopy::version_range< measurement_fields::delta,     0    >
, nocopy::version_range< measurement_fields::first,     0, 1 >
, nocopy::version_range< measurement_fields::coords,    0    >
, nocopy::version_range< measurement_fields::locations, 1    >
, nocopy::version_range< measurement_fields::second,    2, 4 >
, nocopy::version_range< measurement_fields::first,     3, 5 >
>;

SCENARIO("archive") {
  GIVEN("a versioned archive") {
    namespace m = measurement_fields;

    THEN("the correct fields should be present") {
      REQUIRE(measurement<0>::has<m::delta>());
      REQUIRE(measurement<0>::has<m::first>());
      REQUIRE(!measurement<0>::has<m::second>());
      REQUIRE(measurement<0>::has<m::coords>());
      REQUIRE(!measurement<0>::has<m::locations>());

      REQUIRE(measurement<1>::has<m::delta>());
      REQUIRE(measurement<1>::has<m::first>());
      REQUIRE(!measurement<1>::has<m::second>());
      REQUIRE(measurement<1>::has<m::coords>());
      REQUIRE(measurement<1>::has<m::locations>());

      REQUIRE(measurement<2>::has<m::delta>());
      REQUIRE(!measurement<2>::has<m::first>());
      REQUIRE(measurement<2>::has<m::second>());
      REQUIRE(measurement<2>::has<m::coords>());
      REQUIRE(measurement<2>::has<m::locations>());

      REQUIRE(measurement<3>::has<m::delta>());
      REQUIRE(measurement<3>::has<m::first>());
      REQUIRE(measurement<3>::has<m::second>());
      REQUIRE(measurement<3>::has<m::coords>());
      REQUIRE(measurement<3>::has<m::locations>());

      REQUIRE(measurement<4>::has<m::delta>());
      REQUIRE(measurement<4>::has<m::first>());
      REQUIRE(measurement<4>::has<m::second>());
      REQUIRE(measurement<4>::has<m::coords>());
      REQUIRE(measurement<4>::has<m::locations>());

      REQUIRE(measurement<5>::has<m::delta>());
      REQUIRE(measurement<5>::has<m::first>());
      REQUIRE(!measurement<5>::has<m::second>());
      REQUIRE(measurement<5>::has<m::coords>());
      REQUIRE(measurement<5>::has<m::locations>());
    }
  }
}
