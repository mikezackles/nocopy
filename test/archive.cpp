#include <catch.hpp>

#include <nocopy.hpp>

namespace measurement_fields {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_ARRAY(third, int8_t, 4);
  NOCOPY_ARRAY(coords, uint8_t, 10);
  NOCOPY_ARRAY(locations, uint32_t, 20);
}

//                             field removed in this version
//                            field added in this version  |
template <std::size_t Version> //                       |  |
using measurement = //                                  |  |
nocopy::archive_t< //                                   |  |
  Version //                                            v  v
, nocopy::version_range< measurement_fields::delta,     0    >
, nocopy::version_range< measurement_fields::first,     0, 1 >
, nocopy::version_range< measurement_fields::first,     3, 5 >
, nocopy::version_range< measurement_fields::second,    2, 4 >
, nocopy::version_range< measurement_fields::coords,    0    >
, nocopy::version_range< measurement_fields::locations, 1    >
>;

SCENARIO("archive") {
  GIVEN("a versioned archive") {
    namespace m = measurement_fields;
    measurement<0> measure0{};
    measurement<1> measure1{};
    measurement<2> measure2{};
    measurement<3> measure3{};
    measurement<4> measure4{};
    measurement<5> measure5{};

    THEN("the correct fields should be present") {
      REQUIRE(measure0.has<m::delta>());
      REQUIRE(measure0.has<m::first>());
      REQUIRE(!measure0.has<m::second>());
      REQUIRE(measure0.has<m::coords>());
      REQUIRE(!measure0.has<m::locations>());

      REQUIRE(measure1.has<m::delta>());
      REQUIRE(measure1.has<m::first>());
      REQUIRE(!measure1.has<m::second>());
      REQUIRE(measure1.has<m::coords>());
      REQUIRE(measure1.has<m::locations>());

      REQUIRE(measure2.has<m::delta>());
      REQUIRE(!measure2.has<m::first>());
      REQUIRE(measure2.has<m::second>());
      REQUIRE(measure2.has<m::coords>());
      REQUIRE(measure2.has<m::locations>());

      REQUIRE(measure3.has<m::delta>());
      REQUIRE(measure3.has<m::first>());
      REQUIRE(measure3.has<m::second>());
      REQUIRE(measure3.has<m::coords>());
      REQUIRE(measure3.has<m::locations>());

      REQUIRE(measure4.has<m::delta>());
      REQUIRE(measure4.has<m::first>());
      REQUIRE(measure4.has<m::second>());
      REQUIRE(measure4.has<m::coords>());
      REQUIRE(measure4.has<m::locations>());

      REQUIRE(measure5.has<m::delta>());
      REQUIRE(measure5.has<m::first>());
      REQUIRE(!measure5.has<m::second>());
      REQUIRE(measure5.has<m::coords>());
      REQUIRE(measure5.has<m::locations>());
    }
  }
}
