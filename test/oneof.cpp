#include <catch.hpp>

#include <nocopy.hpp>

namespace abc_fields {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
}
using any = nocopy::oneof<
  uint8_t
, abc_fields::a
, abc_fields::b
, abc_fields::c
>;

SCENARIO("oneof") {
  any r{};
  r.get<abc_fields::a>() = 4.5;
  bool a_was_visited = false;
  r.visit(
    [&](abc_fields::a, float val) {
      a_was_visited = true;
      REQUIRE(val == Approx(4.5));
    }
  , [](abc_fields::b, uint32_t) {}
  , [](abc_fields::c, uint8_t) {}
  );
  REQUIRE(a_was_visited);
}
