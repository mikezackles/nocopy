#include <catch.hpp>

#include <nocopy.hpp>

struct abc {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
  using type = nocopy::oneof8<a, b, c>;
};
using abc_t = abc::type;

TEST_CASE("all fields can be set", "[oneof]") {
  abc_t instance{};

  SECTION("first field") {
    instance.get<abc::a>() = 4.5;

    bool a_was_visited = false;
    instance.visit(
      [&](abc::a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    , [](abc::b, uint32_t) {}
    , [](abc::c, uint8_t) {}
    );
    REQUIRE(a_was_visited);
  }

  SECTION("second field") {
    instance.get<abc::b>() = 23;

    bool b_was_visited = false;
    instance.visit(
      [](abc::a, float) {}
    , [&](abc::b, uint32_t val) {
        b_was_visited = true;
        REQUIRE(val == 23);
      }
    , [](abc::c, uint8_t) {}
    );
    REQUIRE(b_was_visited);
  }

  SECTION("third field") {
    instance.get<abc::c>() = 2;

    bool c_was_visited = false;
    instance.visit(
      [](abc::a, float) {}
    , [](abc::b, uint32_t) {}
    , [&](abc::c, uint8_t val) {
        c_was_visited = true;
        REQUIRE(val == 2);
      }
    );
    REQUIRE(c_was_visited);
  }
}

TEST_CASE("visitation lambda order does not matter", "[oneof]") {
  abc_t instance{};
  instance.get<abc::a>() = 4.5;

  SECTION("visitors are passed in the same order as the fields") {
    bool a_was_visited = false;
    instance.visit(
      [&](abc::a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    , [](abc::b, uint32_t) {}
    , [](abc::c, uint8_t) {}
    );
    REQUIRE(a_was_visited);
  }
  SECTION("visitors are passed in a different order than the fields") {
    bool a_was_visited = false;
    instance.visit(
      [](abc::b, uint32_t) {}
    , [](abc::c, uint8_t) {}
    , [&](abc::a, float val) {
        a_was_visited = true;
        REQUIRE(val == Approx(4.5));
      }
    );
    REQUIRE(a_was_visited);
  }
}

SCENARIO("value/zero-initialized oneof has a sensible default", "[oneof]") {
  abc_t instance{};

  bool a_was_visited = false;
  instance.visit(
    [&](abc::a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(0));
    }
  , [](abc::b, uint32_t) {}
  , [](abc::c, uint8_t) {}
  );
  REQUIRE(a_was_visited);
}

struct abcd {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
  NOCOPY_FIELD(d, uint32_t);
  using type = nocopy::oneof8<a, b, c, d>;
};
using abcd_t = abcd::type;

SCENARIO("can contain the same type more than once", "[oneof]") {
  abcd_t instance{};
  SECTION("first repeated type") {
    instance.get<abcd::b>() = 8;

    bool b_was_visited = false;
    instance.visit(
      [&](abcd::b, uint32_t val) {
        b_was_visited = true;
        REQUIRE(val == 8);
      }
    , [](abcd::c, uint8_t) {}
    , [&](abcd::d, uint32_t) {
        b_was_visited = false;
      }
    , [](abcd::a, float) {}
    );
    REQUIRE(b_was_visited);
  }

  SECTION("second repeated type") {
    instance.get<abcd::d>() = 13;

    bool d_was_visited = false;
    instance.visit(
      [&](abcd::b, uint32_t) {
        d_was_visited = false;
      }
    , [](abcd::c, uint8_t) {}
    , [&](abcd::d, uint32_t val) {
        d_was_visited = true;
        REQUIRE(val == 13);
      }
    , [](abcd::a, float) {}
    );
    REQUIRE(d_was_visited);
  }
}

SCENARIO("reassignment changes the visited type", "[oneof]") {
  abcd_t instance{};

  instance.get<abcd::b>() = 8;
  instance.get<abcd::a>() = 3.14f;

  bool a_was_visited = false;
  instance.visit(
    [&](abcd::b, uint32_t) {
      a_was_visited = false;
    }
  , [](abcd::c, uint8_t) {}
  , [](abcd::d, uint32_t) {}
  , [&](abcd::a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(3.14));
    }
  );
  REQUIRE(a_was_visited);
}
