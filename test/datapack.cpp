#include <catch.hpp>

#include <nocopy/datapack.hpp>

NOCOPY_FIELD(delta, float);
NOCOPY_FIELD(first, uint32_t);
NOCOPY_FIELD(second, uint8_t);
NOCOPY_ARRAY(coords, uint8_t, 10);
NOCOPY_ARRAY(locations, uint32_t, 20);
using measurement = nocopy::datapack<0, delta, first, second, coords, locations>;

SCENARIO("datapack") {
  measurement measured{};
  measured.set<delta>(0.5);
  measured.set<first>(1001);
  measured.set<second>(4);
  measured.get<coords>()[4] = 5;
  measured.get<locations>().set(12, 42);
  REQUIRE(measured.get<delta>() == Approx(0.5));
  REQUIRE(measured.get<first>() == 1001);
  REQUIRE(measured.get<second>() == 4);
  REQUIRE(measured.get<coords>()[4] == 5);
  REQUIRE(measured.get<locations>().get(12) == 42);
}
