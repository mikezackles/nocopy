#include <catch.hpp>

#include <nocopy.hpp>

namespace abc_fields {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
}
using abc = nocopy::oneof8<
  abc_fields::a
, abc_fields::b
, abc_fields::c
>;

TEST_CASE("all fields can be set", "[oneof]") {
  abc instance{};

  SECTION("first field") {
    using namespace abc_fields;
    instance.get<a>() = 4.5;

    bool a_was_visited = false;
    instance.visit(
      [&](a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    , [](b, uint32_t) {}
    , [](c, uint8_t) {}
    );
    REQUIRE(a_was_visited);
  }

  SECTION("second field") {
    using namespace abc_fields;
    instance.get<b>() = 23;

    bool b_was_visited = false;
    instance.visit(
      [](a, float) {}
    , [&](b, uint32_t val) {
        b_was_visited = true;
        REQUIRE(val == 23);
      }
    , [](c, uint8_t) {}
    );
    REQUIRE(b_was_visited);
  }

  SECTION("third field") {
    using namespace abc_fields;
    instance.get<c>() = 2;

    bool c_was_visited = false;
    instance.visit(
      [](a, float) {}
    , [](b, uint32_t) {}
    , [&](c, uint8_t val) {
        c_was_visited = true;
        REQUIRE(val == 2);
      }
    );
    REQUIRE(c_was_visited);
  }
}

TEST_CASE("visitation lambda order does not matter", "[oneof]") {
  using namespace abc_fields;

  abc instance{};
  instance.get<a>() = 4.5;

  SECTION("visitors are passed in the same order as the fields") {
    bool a_was_visited = false;
    instance.visit(
      [&](a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    , [](b, uint32_t) {}
    , [](c, uint8_t) {}
    );
    REQUIRE(a_was_visited);
  }
  SECTION("visitors are passed in a different order than the fields") {
    bool a_was_visited = false;
    instance.visit(
      [](b, uint32_t) {}
    , [](c, uint8_t) {}
    , [&](a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    );
    REQUIRE(a_was_visited);
  }
}

SCENARIO("value/zero-initialized oneof has a sensible default", "[oneof]") {
  using namespace abc_fields;
  abc instance{};

  bool a_was_visited = false;
  instance.visit(
    [&](a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(0));
    }
  , [](b, uint32_t) {}
  , [](c, uint8_t) {}
  );
  REQUIRE(a_was_visited);
}

namespace abcd_fields {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
  NOCOPY_FIELD(d, uint32_t);
}
using abcd = nocopy::oneof8<
  abcd_fields::a
, abcd_fields::b
, abcd_fields::c
, abcd_fields::d
>;

SCENARIO("can contain the same type more than once", "[oneof]") {
  using namespace abcd_fields;
  abcd instance{};
  SECTION("first repeated type") {
    instance.get<b>() = 8;

    bool b_was_visited = false;
    instance.visit(
      [&](b, uint32_t val) {
        b_was_visited = true;
        REQUIRE(val == 8);
      }
    , [](c, uint8_t) {}
    , [&](d, uint32_t) {
        b_was_visited = false;
      }
    , [](a, float) {}
    );
    REQUIRE(b_was_visited);
  }

  SECTION("second repeated type") {
    instance.get<d>() = 13;

    bool d_was_visited = false;
    instance.visit(
      [&](b, uint32_t) {
        d_was_visited = false;
      }
    , [](c, uint8_t) {}
    , [&](d, uint32_t val) {
        d_was_visited = true;
        REQUIRE(val == 13);
      }
    , [](a, float) {}
    );
    REQUIRE(d_was_visited);
  }
}

SCENARIO("reassignment changes the visited type", "[oneof]") {
  using namespace abcd_fields;
  abcd instance{};

  instance.get<b>() = 8;
  instance.get<a>() = 3.14f;

  bool a_was_visited = false;
  instance.visit(
    [&](b, uint32_t) {
      a_was_visited = false;
    }
  , [](c, uint8_t) {}
  , [](d, uint32_t) {}
  , [&](a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(3.14));
    }
  );
  REQUIRE(a_was_visited);
}
