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
using measurement = nocopy::datapack<
  measurement_fields::delta
, measurement_fields::first
, measurement_fields::second
, measurement_fields::coords
, measurement_fields::locations
>;

namespace experiment_fields {
  NOCOPY_FIELD(measure1, measurement);
  NOCOPY_ARRAY(more_measurements, measurement, 5);
}
using experiment = nocopy::datapack<
  experiment_fields::measure1
, experiment_fields::more_measurements
>;

SCENARIO("datapack") {
  GIVEN("a datapack-sized buffer of garbage data") {
    using namespace measurement_fields;
    std::array<uint8_t, sizeof(measurement)> buffer;
    std::fill_n(buffer.begin(), buffer.size(), 1);

    WHEN("a default-initialized datapack is constructed from the buffer") {
      auto measurep = new (&buffer) measurement;

      THEN("the data is still garbage") {
        REQUIRE(measurep->get<second>() != 0);
      }
    }

    WHEN("a zero-initialized datapack is constructed from the buffer") {
      auto measurep = new (&buffer) measurement();

      THEN("the data is zeroed") {
        REQUIRE(measurep->get<second>() == 0);
      }
    }

    WHEN("a value-initialized datapack is constructed from the buffer") {
      auto measurep = new (&buffer) measurement{};

      THEN("the data is zeroed") {
        REQUIRE(measurep->get<second>() == 0);
      }
    }
  }

  GIVEN("an initialized datapack") {
    using namespace measurement_fields;
    measurement measured{};
    measured.get<delta>() = 0.5;
    measured.get<first>() = 1001;
    measured.get<second>() = 4;
    measured.get<coords>()[4] = 5;
    measured.get<locations>()[12] = 42;

    THEN("it can be unpacked") {
      REQUIRE(measured.get<delta>() == Approx(0.5));
      REQUIRE(measured.get<first>() == 1001);
      REQUIRE(measured.get<second>() == 4);
      REQUIRE(measured.get<coords>()[4] == 5);
      REQUIRE(measured.get<locations>()[12] == 42);
    }

    THEN("its largest alignment divides its size") {
      REQUIRE(sizeof(measurement) % sizeof(uint32_t) == 0);
    }

    THEN("calling get on a scalar field returns a box reference") {
      REQUIRE((std::is_same<decltype(measured.get<first>()), nocopy::box<uint32_t>&>::value));
    }

    THEN("calling get on an array field returns a std::array reference") {
      REQUIRE((std::is_same<decltype(measured.get<coords>()), std::array<uint8_t, 10>&>::value));
    }

    THEN("calling get on an array field returns a std::array reference") {
      REQUIRE((std::is_same<decltype(measured.get<coords>()), std::array<uint8_t, 10>&>::value));
    }

    THEN("calling get on an arraypack field returns an arraypack reference") {
      REQUIRE((std::is_same<decltype(measured.get<locations>()), std::array<nocopy::box<uint32_t>, 20>&>::value));
    }

    WHEN("a const reference is taken") {
      auto const& cmeasured = measured;

      THEN("it can be unpacked") {
        REQUIRE(cmeasured.get<delta>() == Approx(0.5));
        REQUIRE(cmeasured.get<first>() == 1001);
        REQUIRE(cmeasured.get<second>() == 4);
        REQUIRE(cmeasured.get<coords>()[4] == 5);
        REQUIRE(cmeasured.get<locations>()[12] == 42);
      }

      THEN("accessing boxed fields returns a const reference") {
        REQUIRE((std::is_same<decltype(cmeasured.get<delta>()), nocopy::box<float> const&>::value));
      }

      THEN("calling get on an array field returns a const std::array reference") {
        REQUIRE((std::is_same<decltype(cmeasured.get<coords>()), std::array<uint8_t, 10> const&>::value));
      }

      THEN("calling get on an arraypack field returns a const arraypack reference") {
        REQUIRE((std::is_same<decltype(cmeasured.get<locations>()), std::array<nocopy::box<uint32_t>, 20> const&>::value));
      }
    }
  }

  GIVEN("a datapack with nested datapacks") {
    namespace e = experiment_fields;
    namespace m = measurement_fields;
    experiment exp{};

    exp.get<e::measure1>().get<m::second>() = 5;
    exp.get<e::more_measurements>()[4].get<m::second>() = 12;

    THEN("it can be unpacked") {
      REQUIRE(exp.get<e::measure1>().get<m::second>() == 5);
      REQUIRE(exp.get<e::more_measurements>()[4].get<m::second>() == 12);
    }
  }
}
