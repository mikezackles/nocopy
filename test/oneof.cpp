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
    instance.get<abc_fields::a>() = 4.5;

    bool a_was_visited = false;
    instance.visit(
      [&](abc_fields::a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    , [](abc_fields::b, uint32_t) {}
    , [](abc_fields::c, uint8_t) {}
    );
    REQUIRE(a_was_visited);
  }

  SECTION("second field") {
    instance.get<abc_fields::b>() = 23;

    bool b_was_visited = false;
    instance.visit(
      [](abc_fields::a, float) {}
    , [&](abc_fields::b, uint32_t val) {
        b_was_visited = true;
        REQUIRE(val == 23);
      }
    , [](abc_fields::c, uint8_t) {}
    );
    REQUIRE(b_was_visited);
  }

  SECTION("third field") {
    instance.get<abc_fields::c>() = 2;

    bool c_was_visited = false;
    instance.visit(
      [](abc_fields::a, float) {}
    , [](abc_fields::b, uint32_t) {}
    , [&](abc_fields::c, uint8_t val) {
        c_was_visited = true;
        REQUIRE(val == 2);
      }
    );
    REQUIRE(c_was_visited);
  }
}

TEST_CASE("visitation lambda order does not matter", "[oneof]") {
  abc instance{};
  instance.get<abc_fields::a>() = 4.5;

  SECTION("visitors are passed in the same order as the fields") {
    bool a_was_visited = false;
    instance.visit(
      [&](abc_fields::a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    , [](abc_fields::b, uint32_t) {}
    , [](abc_fields::c, uint8_t) {}
    );
    REQUIRE(a_was_visited);
  }
  SECTION("visitors are passed in a different order than the fields") {
    bool a_was_visited = false;
    instance.visit(
      [](abc_fields::b, uint32_t) {}
    , [](abc_fields::c, uint8_t) {}
    , [&](abc_fields::a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    );
    REQUIRE(a_was_visited);
  }
}

SCENARIO("value/zero-initialized oneof has a sensible default", "[oneof]") {
  abc instance{};

  bool a_was_visited = false;
  instance.visit(
    [&](abc_fields::a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(0));
    }
  , [](abc_fields::b, uint32_t) {}
  , [](abc_fields::c, uint8_t) {}
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
  abcd instance{};
  SECTION("first repeated type") {
    instance.get<abcd_fields::b>() = 8;

    bool b_was_visited = false;
    instance.visit(
      [&](abcd_fields::b, uint32_t val) {
        b_was_visited = true;
        REQUIRE(val == 8);
      }
    , [](abcd_fields::c, uint8_t) {}
    , [&](abcd_fields::d, uint32_t) {
        b_was_visited = false;
      }
    , [](abcd_fields::a, float) {}
    );
    REQUIRE(b_was_visited);
  }

  SECTION("second repeated type") {
    instance.get<abcd_fields::d>() = 13;

    bool d_was_visited = false;
    instance.visit(
      [&](abcd_fields::b, uint32_t) {
        d_was_visited = false;
      }
    , [](abcd_fields::c, uint8_t) {}
    , [&](abcd_fields::d, uint32_t val) {
        d_was_visited = true;
        REQUIRE(val == 13);
      }
    , [](abcd_fields::a, float) {}
    );
    REQUIRE(d_was_visited);
  }
}

SCENARIO("reassignment changes the visited type", "[oneof]") {
  abcd instance{};

  instance.get<abcd_fields::b>() = 8;
  instance.get<abcd_fields::a>() = 3.14f;

  bool a_was_visited = false;
  instance.visit(
    [&](abcd_fields::b, uint32_t) {
      a_was_visited = false;
    }
  , [](abcd_fields::c, uint8_t) {}
  , [](abcd_fields::d, uint32_t) {}
  , [&](abcd_fields::a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(3.14));
    }
  );
  REQUIRE(a_was_visited);
}
