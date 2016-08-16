Overview
-

`nocopy` is a header-only library that aims to provide a simple, type-safe,
compile-time, zero-copy solution to serialization. As of this writing, the
[`include`](include/) directory contains less than 1500 lines of code.

This project was inspired by
[FlatBuffers](https://google.github.io/flatbuffers/) and
[Cap'n Proto](https://capnproto.org/). Unlike those projects, `nocopy` makes no
attempt to directly support any languages other than C++14, and therefore there
is no standalone schema compiler. Your C++ compiler is your schema compiler.

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

Structs ([`nocopy::datapack`](test/datapack.cpp))
-

`nocopy` datapacks are essentially traditional packed structs, but the packing
is performed automatically at compile time. Scalar types are aligned to their
size, and datapack fields are sorted by alignment from largest to smallest.
Because these alignments are all guaranteed to be powers of two, smaller
alignments divide larger ones. This ensures that if the datapack itself is
aligned to the alignment of the largest field, all its members are aligned.
Padding is inserted to align the end of the datapack with the largest alignment
it contains. This ensures that datapacks can contain other datapacks as nested
fields without affecting the alignment of subsequent fields.

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
  using type = nocopy::datapack<delta, first, second, coords, locations>;
};

struct experiment {
  NOCOPY_FIELD(measure1, measurement::type);
  NOCOPY_FIELD(more_measurements, NOCOPY_ARRAY(measurement::type, 5));
  using type = nocopy::datapack<measure1, more_measurements>;
};

int main() {
  experiment::type exp{};

  exp.get<experiment::measure1>().get<measurement::second>() = 5;
  exp.get<experiment::more_measurements>()[4].get<measurement::second>() = 12;

  std::cout
    << exp.get<experiment::measure1>().get<measurement::second>()
    << " == 5"
    << std::endl;
  std::cout
    << exp.get<experiment::more_measurements>()[4].get<measurement::second>()
    << " == 12"
    << std::endl;

  return 0;
}
```

Note that the macros are just syntactic sugar, and if you prefer not to use
them, their definitions are as follows:

```c++
#define NOCOPY_FIELD(field_name, type) \
  struct field_name { \
    using field_type = type; \
    static constexpr auto name() { return #field_name; } \
  }

#define NOCOPY_ARRAY(type, size) ::nocopy::array<type, size>
#define NOCOPY_ONEOF(...) ::nocopy::oneof8<__VA_ARGS__>
#define NOCOPY_ONEOF16(...) ::nocopy::oneof16<__VA_ARGS__>
```

Technically, `field_name::name` is currently unused, but it is intended to
support JSON dumps.

Unions ([`nocopy::oneof`](test/oneof.cpp))
-

```c++
#include <nocopy.hpp>
#include <iostream>

struct abc {
  NOCOPY_FIELD(a, float);
  NOCOPY_FIELD(b, uint32_t);
  NOCOPY_FIELD(c, uint8_t);
  using type = nocopy::oneof8<a, b, c>;
};

int main() {
  abc::type instance{};
  instance.get<abc::a>() = 4.5;
  instance.visit(
    [](abc::a, float val) {
      std::cout << "Val is " << val << " (should be 4.5)" << std::endl;
    }
  , [](abc::b, uint32_t) {
      std::cerr << "This shouldn't happen" << std::endl;
    }
  , [](abc::c, uint8_t) {
      std::cerr << "This shouldn't happen either" << std::endl;
    }
  );
  return 0;
}
```

The field type is passed as the first argument to the visitor to allow for
unions that contain the same type under a different name.

Versioning ([`nocopy::schema_t`](test/archive.cpp))
-

```c++
#include <nocopy.hpp>

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));

  //         field removed in this version
  //        field added in this version  |
  template <std::size_t Version> //   |  |
  using type = //                     |  |
  nocopy::schema_t< //                |  |
    nocopy::datapack //               |  |
  , Version //                        v  v
  , nocopy::version_range< delta,     0    >
  , nocopy::version_range< first,     0, 1 >
  , nocopy::version_range< coords,    0    >
  , nocopy::version_range< locations, 1    >
  , nocopy::version_range< second,    2, 4 >
  , nocopy::version_range< first,     3, 5 >
  >;
};
measurement::type<3> measurement_v3;
```

Dynamic Memory ([`nocopy::heap`](test/heap.cpp))
-

Right now this is mostly a proof of concept, but `nocopy` includes a very basic
heap implementation (not thread safe). So, for example, one system could create
a heap inside a buffer, send the buffer to another system, and that system could
then edit it directly. Please let me know if you find interesting use cases.

For variable byte size heap support, make sure to set the AssumeSameSizedByte
template parameter to false (divides the maximum heap size by `CHAR_BIT`).

Note that a higher level interface may be forthcoming, but for now, clients must
manage heap allocations manually.

```c++
#include <nocopy.hpp>

struct measurement {
  NOCOPY_FIELD(delta, float);
  NOCOPY_FIELD(first, uint32_t);
  NOCOPY_FIELD(second, uint8_t);
  NOCOPY_FIELD(third, NOCOPY_ARRAY(int8_t, 4));
  NOCOPY_FIELD(coords, NOCOPY_ARRAY(uint8_t, 10));
  NOCOPY_FIELD(locations, NOCOPY_ARRAY(uint32_t, 20));
  using type = nocopy::datapack<delta, first, second, coords, locations>;
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
  m[1].get<measurement::first>() = 2000;
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

Assumptions
-

* std::array is POD and `sizeof(std::array<unsigned char, N>) == N`
* floating point numbers are in IEEE 754 format, float contains 32 bits, and
 double contains 64 bits

`nocopy` does its best to check assumptions at compile time, so if for some reason
your platform is not compatible, your code should fail to compile. Note that
features that are not used are not checked, so for example if you intend to
support platforms with exotic floating point implementations, you can still
compile if your code contains no floating point fields.

Note that `CHAR_BIT == 8` is *not* an assumption, so as of now, nocopy attempts
to support platforms where a byte contains more than 8 bits. For now this
support is theoretical as I have no such hardware on which to test.

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

This project is still in flux, and although it aims to generate a stable,
portable binary encoding, it may still fall short of that goal. Use at your own
risk, and please report bugs!

For the Future
-

* Dump to JSON
* Consider converting exotic floating point implementations to IEEE 754 using a
  boxing approach similar to that used for endianness.
* Investigate supporting [Brigand](https://github.com/edouarda/brigand) as an
  alternative to Boost.Hana.
* Stack allocation
* Framing
* Consider a migration-like schema instead of version ranges
* Consider migrating heap corruption test to
  [rapidcheck](https://github.com/emil-e/rapidcheck)
