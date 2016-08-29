Overview
-

`nocopy` is a header-only library that aims to provide a type-safe,
compile-time, zero-copy solution to serialization. As of this writing, the
[`include`](include/) directory contains less than 1700 lines of code.

### Similar projects

This project was inspired by
[FlatBuffers](https://google.github.io/flatbuffers/) and
[Cap'n Proto](https://capnproto.org/). Unlike those projects, `nocopy` makes no
attempt to directly support any languages other than C++14, and therefore there
is no standalone schema compiler. Your C++ compiler is your schema compiler.

If you don't want zero copy, I suggest taking a look at the excellent
[cereal](http://uscilab.github.io/cereal/) library.

Callbacks
-

All runtime error handling and visitation in `nocopy` is performed via lambda
callbacks. Functions that may result in multiple types and/or a
`std::error_code` accept type-safe lambas (in any order). The values passed to
these lambas can be handled inline, or they may be merged into a single type via
return type deduction. For example, you might handle an error by returning a
valid result, or you could simply throw an exception.

###Inline
```c++
some_nocopy_function(
  [](valid_type1 t1) {
    t1.do_something();
  }
, [](valid_type2 t2) {
    t2.do_something();
  }
, [](std::error_code e) {
    std::cerr << e.message() << std::endl;
  }
);
```

###Bubble Up
```c++
int result = some_nocopy_function(
  [](valid_type1 t1) {
    return t1.do_something();
  }
, [](valid_type2 t2) {
    return t2.do_something();
  }
, [](std::error_code e) {
    std::cerr << e.message() << std::endl;
    return 5;
  }
);
std::cout << "result is " << result << std::endl;
```

###Ignore
```c++
int result = some_nocopy_function(
  [](valid_type1 t1) {
    return t1.do_something();
  }
, [](valid_type2 t2) {
    return t2.do_something();
  }
, [](std::error_code e) -> int {
    throw std::runtime_error{e.message()};
  }
);
std::cout << "result is " << result << std::endl;
```

[structpack](test/structpack.cpp)
-

`nocopy` structpacks are essentially traditional packed structs, but the packing
is performed automatically at compile time. Scalar types are aligned to their
size, and structpack fields are sorted by alignment from largest to smallest.
Because these alignments are all guaranteed to be powers of two, smaller
alignments divide larger ones. This ensures that if the structpack itself is
aligned to the alignment of the largest field, all its members are aligned.
Padding is inserted to align the end of the structpack with the largest
alignment it contains. This ensures that structpacks can contain other
structpacks as nested fields without affecting the alignment of subsequent
fields.

```c++
#include <nocopy.hpp>
#include <iostream>

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

int main() {
  using m = measurement;
  using e = experiment;

  e::type exp{};

  exp[e::measure1][m::second] = 5;
  exp[e::more_ms][4][m::second] = 12;

  std::cout
    << exp[e::measure1][m::second]
    << " == 5"
    << std::endl;
  std::cout
    << exp[e::more_measurements][4][m::second]
    << " == 12"
    << std::endl;

  return 0;
}
```

Note that the macros are just syntactic sugar, and if you prefer not to use
them, their definitions are as follows:

```c++
#define NOCOPY_FIELD(field_name, type) \
  struct field_name ## _t { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  }; \
  static constexpr field_name ## _t field_name{}

// This one is for nested schemas (see below)
#define NOCOPY_VERSIONED_FIELD(field_name, type) \
  template <std::size_t Version> \
  struct field_name ## _t { \
    using field_type = typename type ::v<Version>; \
    static constexpr auto name() { return #field_name; } \
  }

#define NOCOPY_ARRAY(type, size) ::nocopy::array<type, size>
#define NOCOPY_ONEOF(...) ::nocopy::oneof8<__VA_ARGS__>
#define NOCOPY_ONEOF16(...) ::nocopy::oneof16<__VA_ARGS__>
```

Technically, `field_name::name` is currently unused, but it is intended to
support JSON dumps (not yet implemented).

[oneof](test/oneof.cpp)
-

`nocopy::oneof8` and `nocopy::oneof16` are tagged union implementations that use
8-bit and 16-bit tags, respectively.

```c++
#include <nocopy.hpp>
#include <iostream>

struct abc {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
  using type = nocopy::oneof8<a_t, b_t, c_t>;
};

int main() {
  abc::type instance{};
  instance[abc::a] = 4.5;
  instance.visit(
    [](abc::a_t, float val) {
      std::cout << "Val is " << val << " (should be 4.5)" << std::endl;
    }
  , [](abc::b_t, uint32_t) {
      std::cerr << "This shouldn't happen" << std::endl;
    }
  , [](abc::c_t, uint8_t) {
      std::cerr << "This shouldn't happen either" << std::endl;
    }
  );
  return 0;
}
```

The field type is passed as the first argument to the visitor to allow for
unions that contain the same type under a different name.

[schema](test/schema.cpp)
-

The `nocopy::schema` type facilitates versioned structpacks. Technically this
type contains a nested structpack in its `data` member, but it supports the same
structpack interface, with additional member functions dedicated to nested
schemas. This nested schema handling allows a single schema version to propagate
to nested types automatically.

```c++
#include <nocopy.hpp>

struct nested {
  NOCOPY_FIELD(a, uint16_t);
  NOCOPY_FIELD(b, int8_t);
  NOCOPY_FIELD(c, int32_t);
  NOCOPY_FIELD(d, int32_t);
  NOCOPY_FIELD(e, NOCOPY_ONEOF(c_t, d_t));

  template <std::size_t Version>
  using v =
  nocopy::schema<
    Version
  , nocopy::version_range<a_t, 4>
  , nocopy::version_range<b_t, 4, 4>
  , nocopy::version_range<c_t, 4>
  , nocopy::version_range<e_t, 5>
  >;
};

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));
  NOCOPY_VERSIONED_FIELD(nested_field, nested);

  //                     field removed in this version
  //                    field added in this version  |
  template <std::size_t Version> //               |  |
  using v = //                                    |  |
  nocopy::schema< //                              |  |
    Version //                                    v  v
  , nocopy::version_range< delta_t,               0    >
  , nocopy::version_range< first_t,               0, 1 >
  , nocopy::version_range< coords_t,              0    >
  , nocopy::version_range< locations_t,           1    >
  , nocopy::version_range< second_t,              2, 4 >
  , nocopy::version_range< first_t,               3, 5 >
  , nocopy::version_range< nested_field<Version>, 4, 5 >
  >;
};

measurement::v<3> measurement_v3{};
measurement_v3.get<measurement::nested_field>()[nested::a] = 4;
```

Note that accessing schema fields that refer to another schema requires a
slightly different interface from normal fields (`schema::get` instead of
`schema::operator[]`. This is because the type passed is incomplete. (It
requires a version.)

[heap](test/heap.cpp)
-

Right now this is mostly a proof of concept, but `nocopy` includes a very basic
heap implementation (not thread safe). So, for example, one system could create
a heap inside a buffer, send the buffer to another system, and that system could
then edit it directly. At some point I'd like to make heap reference deletes
cascade, in that deleting a reference owning other references would
automatically trigger the children's deletion.

For variable byte size heap support, make sure to set the AssumeSameSizedByte
template parameter to false (divides the maximum heap size by `CHAR_BIT`).

```c++
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

int main() {
  std::array<unsigned char, 1_KB> buffer;
  auto heap = nocopy::heap64::create(
    buffer.data(), sizeof(buffer)
  , [](auto heap) { return heap; }
  , [](std::error_code) -> nocopy::heap64 {
      throw std::runtime_error{"shouldn't happen"};
    }
  );
  auto result = heap.malloc_range<measurement::type>(
    2 // allocate space for 2 instances
  , [](auto result) { return result; }
  , [](std::error_code) -> nocopy::heap64::range_reference<measurement::type> {
      throw std::runtime_error{"shouldn't happen"};
    }
  );
  auto m = heap.deref(result);
  m[1][measurement::first] = 2000;
  heap.free(result);
  return 0;
}
```

Platforms
-

`nocopy` uses [Boost.Hana](https://github.com/boostorg/hana), which requires
relatively strict C++14 compliance. As such, nocopy is limited to the platforms
on which Boost.Hana compiles (current versions of gcc or clang, including
clang-cl). For now, my tests are limited to these two compilers on Arch Linux.

Assumptions/Caveats
-

* `sizeof(std::array<unsigned char, N>) == N`
* floating point numbers are in IEEE 754 format, `float` contains 32 bits, and
 `double` contains 64 bits
* `long double` fields are not supported

`nocopy` does its best to check assumptions at compile time, so if for some
reason your platform is not compatible, your code should fail to compile. Note
that features that are not used are not checked, so for example if you intend to
support platforms with exotic floating point implementations, you can still
compile if your code contains no floating point fields.

Note that `CHAR_BIT == 8` is *not* an assumption, so as of now, `nocopy`
attempts to support platforms where a byte contains more than 8 bits. For now
this support is theoretical as I have no such hardware on which to test.

There is not yet support for a map/hash type.

Strict Aliasing
-

`nocopy` uses `reinterpret_cast`, but special care has been taken to avoid
strict aliasing errors. Specifically, `reinterpret_cast` is only used to convert
references to `unsigned char` to references to aggregate types whose sole data
member is `std::array<unsigned char, N>`. It is also used to cast scalar types
to `unsigned char` during endian conversion. I've done my best to interpret the
standard, but I am not a language lawyer, so please let me know if I've
misunderstood something here.

Optimization
-

As data is stored in little-endian format, no conversion is necessary on
little-endian platforms. AFAIK, there is no reliable way to detect endianness at
compile time, so by default, `nocopy` performs conversion on all platforms. Your
compiler may be smart enough to optimize this away, but if you want to be
certain that no conversion is performed, simply define
`NOCOPY_OPTIMIZE_LITTLE_ENDIAN` (`OPTIMIZE_LITTLE_ENDIAN` in the CMake cache).
Note that if you mistakenly compile with this defined on big-endian platforms,
the serialized data will not be portable. This flag may also be used to turn off
conversion regardless of platform if you aren't worried about portability.

Disclaimer
-

**This project is still in flux**, and although it aims to generate a stable,
portable binary encoding, it may still fall short of that goal. Use at your own
risk, and please report bugs!

For the Future
-

* Dump to JSON
* Investigate supporting [Brigand](https://github.com/edouarda/brigand) as an
  alternative to Boost.Hana.
* Framing
* Consider a migration-like schema instead of version ranges
* Consider simulated constructors/destructors for dynamic allocations

License
-

`nocopy` is distributed under the [Apache License, Version 2.0](LICENSE.txt)
