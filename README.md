This project is still in flux, and although it aims to provide a flexible,
portable, zero-copy binary encoding without using a standalone schema compiler,
it may still fall short of that goal. Please report bugs!

Structs (`nocopy::datapack`)
-

Nocopy datapacks are essentially traditional packed structs, but the packing is
performed automatically at compile time. Scalar types are aligned to their size,
and datapack fields are sorted by alignment from largest to smallest, which
ensures that if the datapack itself is aligned to the alignment of the largest
field, all its members are aligned. Padding is inserted to align the end of the
datapack with the largest alignment it contains. This ensures that datapacks can
contain other datapacks as nested fields without affecting the alignment of
subsequent fields.

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
using measurement_t = measurement::type;

struct experiment {
  NOCOPY_FIELD(measure1, measurement_t);
  NOCOPY_FIELD(more_measurements, NOCOPY_ARRAY(measurement_t, 5));
  using type = nocopy::datapack<measure1, more_measurements>;
};
using experiment_t = experiment::type;

int main() {
  experiment_t exp{};

  exp.get<experiment::measure1>().get<measurement::second>() = 5;
  exp.get<experiment::more_measurements>()[4].get<measurement::second>() = 12;

  std::cout << exp.get<experiment::measure1>().get<measurement::second>() << " == 5" << std::endl;
  std::cout << exp.get<experiment::more_measurements>()[4].get<measurement::second>() << " == 12" << std::endl;

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

Technically, `field_name::name` is currently unused, but it may be used in the
future for doing things like dumping to JSON.

Unions (`nocopy::oneof`)
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
using abc_t = abc::type;

int main() {
  abc_t instance{};
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

Note that the lambdas passed to `abc_t::visit` can appear in any order, and they
are always passed by value. This is generally true for lambdas passed as
callbacks to nocopy.

Archives
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
  nocopy::archive_t< //               |  |
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
template <std::size_t Version>
using measurement_t = typename measurement::type<Version>;
```

Heap
-

Nocopy includes a very basic heap implementation (not thread safe). So, for
example, one system could create a heap inside a buffer, send the buffer to
another system, and that system could then edit it directly. There may also be
interesting possibilities here using COW filesystems such as btrfs.

A higher level interface may be forthcoming, but for now, clients must manage
heap allocations manually.

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
  using type = nocopy::datapack<delta, first, second, coords, locations>;
};
using measurement_t = measurement::type;

int main() {
  std::array<unsigned char, 1_KB> buffer;
  nocopy::heap64::create(&buffer[0], sizeof(buffer)
  , [](nocopy::heap64 heap) {
      heap.malloc(2*sizeof(measurement_t)
      , [heap](nocopy::heap64::offset_t result) mutable {
          auto m = heap.deref<measurement_t>(result);
          m++;
          m->get<measurement::first>() = 2000;
          heap.free(result);
        }
      , [](std::error_code) {}
      );
    }
  , [](std::error_code) {}
  );
  return 0;
}
```

Platforms
-

Nocopy uses [Boost.Hana](https://github.com/boostorg/hana), which requires
relatively strict C++14 compliance. As such, nocopy is limited to the platforms
on which Boost.Hana compiles (current versions of gcc or clang, including
clang-cl). For now, my tests are limited to these two compilers on Arch Linux.

Assumptions
-

* std::array is POD and `sizeof(std::array<unsigned char, N>) == N`
* floating point numbers are in IEEE 754 format, float contains 32 bits, and
  double contains 64 bits

Nocopy does its best to check assumptions at compile time, so if for some reason
your platform is not compatible, your code should fail to compile. Note that
features that are not used are not checked, so for example if you intend to
support platforms with exotic floating point implementations, you can still
compile if your code contains no floating point fields.

Note that `CHAR_BIT == 8` is *not* an assumption, so as of now, nocopy attempts
to support platforms where a byte contains more than 8 bits. For now this
support is theoretical as I have no such hardware on which to test.

Strict Aliasing
-

Nocopy uses `reinterpret_cast`, but special care has been taken to avoid strict
aliasing errors. Specifically, `reinterpret_cast` is only used to convert
references to unsigned char to references to POD types whose sole data member is
`std::array<unsigned char, N>`. It is also used to cast scalar types to unsigned
char during endian conversion. I am not a language lawyer, so please let me know
if I've misunderstood something here.

Optimization
-

As data is stored in little-endian format, no conversion should be performed on
little-endian platforms. AFAIK, there is no reliable way to detect endianness at
compile time, so by default, nocopy performs conversion on all platforms. Your
compiler may be smart enough to optimize this away, but if you want to be
certain that no conversion is performed, simply define
NOCOPY_OPTIMIZE_LITTLE_ENDIAN (OPTIMIZE_LITTLE_ENDIAN in the cmake cache). Note
that if you mistakenly compile with this defined on big-endian platforms, the
serialized data will not be portable. This flag may also be used to turn off
conversion regardless of platform if you aren't worried about portability.

For the Future
-

* Dump to JSON
* Consider converting exotic floating point implementations to IEEE 754 using a
  boxing approach similar to that used for endianness.
* Investigate supporting [Brigand](https://github.com/edouarda/brigand) as an
  alternative to Boost.Hana.
