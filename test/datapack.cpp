#include <catch.hpp>

#include <nocopy/datapack.hpp>

NOCOPY_FIELD(delta, float);
NOCOPY_FIELD(first, uint32_t);
NOCOPY_FIELD(second, uint8_t);
using measurement = nocopy::datapack<0, delta, first, second>;

SCENARIO("datapack") {
  std::aligned_storage<100, sizeof(uint32_t)> buffer;
  auto measured = measurement::create(&buffer, 100);
  REQUIRE(measured);
  measured->set<delta>(0.5);
  measured->set<first>(1001);
  measured->set<second>(4);
  REQUIRE(measured->get<delta>() == Approx(0.5));
  REQUIRE(measured->get<first>() == 1001);
  REQUIRE(measured->get<second>() == 4);
}
