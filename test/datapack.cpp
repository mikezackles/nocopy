#include <catch.hpp>

#include <nocopy/datapack.hpp>

namespace hana = boost::hana;

NOCOPY_FIELD(delta, float);
NOCOPY_FIELD(first, uint32_t);
NOCOPY_FIELD(second, uint8_t);
NOCOPY_ARRAY(third, int8_t, 4);
NOCOPY_ARRAY(coords, uint8_t, 10);
NOCOPY_ARRAY(locations, uint32_t, 20);
using measurement = nocopy::datapack<delta, first, second, coords, locations>;

//template <std::size_t Version>
//using measurement =
////  field removed in this version
//// field added in this version  |
//nocopy::archive< //          |  |
//  Version //                 v  v
//, nocopy::v< delta,          0    >
//, nocopy::v< first,          0, 1 >
//, nocopy::v< second,         0    >
//, nocopy::v< coords,         0    >
//, nocopy::v< locations,      1    >
//>;

SCENARIO("datapack") {
  GIVEN("an initialized datapack") {
    measurement measured{};
    measured.set<delta>(0.5);
    measured.set<first>(1001);
    measured.set<second>(4);
    measured.get<coords>()[4] = 5;
    measured.get<locations>().set(12, 42);

    THEN("it can be unpacked") {
      REQUIRE(measured.get<delta>() == Approx(0.5));
      REQUIRE(measured.get<first>() == 1001);
      REQUIRE(measured.get<second>() == 4);
      REQUIRE(measured.get<coords>()[4] == 5);
      REQUIRE(measured.get<locations>().get(12) == 42);
    }

    THEN("calling get on a scalar field returns by value") {
      REQUIRE(static_cast<bool>(
        hana::type_c<decltype(measured.get<first>())> == hana::type_c<uint32_t>
      ));
    }

    THEN("calling get on an array field returns a std::array reference") {
      REQUIRE(static_cast<bool>(
        hana::type_c<decltype(measured.get<coords>())> == hana::type_c<std::array<uint8_t, 10>&>
      ));
    }

    THEN("calling get on an array field returns a std::array reference") {
      REQUIRE(static_cast<bool>(
        hana::type_c<decltype(measured.get<coords>())> == hana::type_c<std::array<uint8_t, 10>&>
      ));
    }

    THEN("calling get on an arraypack field returns an arraypack reference") {
      REQUIRE(static_cast<bool>(
        hana::type_c<decltype(measured.get<locations>())> == hana::type_c<nocopy::arraypack<uint32_t, 20>&>
      ));
    }

    GIVEN("a const reference") {
      auto const& cmeasured = measured;

      THEN("it can be unpacked") {
        REQUIRE(cmeasured.get<delta>() == Approx(0.5));
        REQUIRE(cmeasured.get<first>() == 1001);
        REQUIRE(cmeasured.get<second>() == 4);
        REQUIRE(cmeasured.get<coords>()[4] == 5);
        REQUIRE(cmeasured.get<locations>().get(12) == 42);
      }

      THEN("const doesn't leak to by-value types") {
        REQUIRE(static_cast<bool>(
          hana::type_c<decltype(cmeasured.get<delta>())> == hana::type_c<float>
        ));
      }

      THEN("calling get on an array field returns a const std::array reference") {
        REQUIRE(static_cast<bool>(
          hana::type_c<decltype(cmeasured.get<coords>())> == hana::type_c<std::array<uint8_t, 10> const&>
        ));
      }

      THEN("calling get on an arraypack field returns a const arraypack reference") {
        REQUIRE(static_cast<bool>(
          hana::type_c<decltype(cmeasured.get<locations>())> == hana::type_c<nocopy::arraypack<uint32_t, 20> const&>
        ));
      }
    }
  }
}
