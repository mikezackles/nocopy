#include <catch.hpp>

#include <nocopy.hpp>

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));
  using type = nocopy::structpack<delta_t, first_t, second_t, coords_t, locations_t>;
};

struct experiment {
  NOCOPY_FIELD(measure1, measurement::type);
  NOCOPY_FIELD(more_measurements, NOCOPY_ARRAY(measurement::type, 5));
  using type = nocopy::structpack<measure1_t, more_measurements_t>;
};

SCENARIO("structpack") {
  using m = measurement;
  using e = experiment;

  GIVEN("a structpack-sized buffer of garbage data") {
    std::array<uint8_t, sizeof(m::type)> buffer;
    buffer.fill(1);

    // NOTE - These test cases have been disabled because value initialization
    // is required now that nocopy aggregate types are implemented using public
    // const buffers.

    //WHEN("a default-initialized structpack is constructed from the buffer") {
    //  auto measurep = new (&buffer) measurement::type;

    //  THEN("the data is still garbage") {
    //    REQUIRE(measurep->get(measurement::second>) != 0);
    //  }
    //}

    //WHEN("a zero-initialized structpack is constructed from the buffer") {
    //  auto measurep = new (&buffer) measurement::type();

    //  THEN("the data is zeroed") {
    //    REQUIRE(measurep->get(measurement::second>) == 0);
    //  }
    //}

    WHEN("a value-initialized structpack is constructed from the buffer") {
      auto measurep = new (&buffer) m::type{};

      THEN("the data is zeroed") {
        REQUIRE((*measurep)[m::second] == 0);
      }
    }
  }

  GIVEN("an initialized structpack") {
    m::type measured{};

    measured[m::delta] = 0.5;
    measured[m::first] = 1001;
    measured[m::second] = 4;
    measured[m::coords][4] = 5;
    measured[m::locations][12] = 42;

    THEN("it can be unpacked") {
      REQUIRE(measured[m::delta] == Approx(0.5));
      REQUIRE(measured[m::first] == 1001);
      REQUIRE(measured[m::second] == 4);
      REQUIRE(measured[m::coords][4] == 5);
      REQUIRE(measured[m::locations][12] == 42);
    }

    THEN("its largest alignment divides its size") {
      REQUIRE(sizeof(m::type) % sizeof(uint32_t) == 0);
    }

    THEN("calling get on a scalar field returns a box reference") {
      REQUIRE((std::is_same<decltype(measured[m::first]), nocopy::box<uint32_t>&>::value));
    }

    THEN("calling get on an array field returns a std::array reference") {
      REQUIRE((std::is_same<decltype(measured[m::coords]), std::array<uint8_t, 10>&>::value));
    }

    THEN("calling get on an array field returns a std::array reference") {
      REQUIRE((std::is_same<decltype(measured[m::coords]), std::array<uint8_t, 10>&>::value));
    }

    THEN("calling get on an arraypack field returns an arraypack reference") {
      REQUIRE((std::is_same<decltype(measured[m::locations]), std::array<nocopy::box<uint32_t>, 20>&>::value));
    }

    WHEN("a const reference is taken") {
      auto const& cmeasured = measured;

      THEN("it can be unpacked") {
        REQUIRE(cmeasured[m::delta] == Approx(0.5));
        REQUIRE(cmeasured[m::first] == 1001);
        REQUIRE(cmeasured[m::second] == 4);
        REQUIRE(cmeasured[m::coords][4] == 5);
        REQUIRE(cmeasured[m::locations][12] == 42);
      }

      THEN("accessing boxed fields returns a const reference") {
        REQUIRE((std::is_same<decltype(cmeasured[m::delta]), nocopy::box<float> const&>::value));
      }

      THEN("calling get on an array field returns a const std::array reference") {
        REQUIRE((std::is_same<decltype(cmeasured[m::coords]), std::array<uint8_t, 10> const&>::value));
      }

      THEN("calling get on an arraypack field returns a const arraypack reference") {
        REQUIRE((std::is_same<decltype(cmeasured[m::locations]), std::array<nocopy::box<uint32_t>, 20> const&>::value));
      }
    }
  }

  GIVEN("a structpack with nested structpacks") {
    e::type exp{};

    exp[e::measure1][m::second] = 5;
    exp[e::more_measurements][4][m::second] = 12;

    THEN("it can be unpacked") {
      REQUIRE(exp[e::measure1][m::second] == 5);
      REQUIRE(exp[e::more_measurements][4][m::second] == 12);
    }
  }
}
