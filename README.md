This project is highly experimental, and although it aims to provide a flexible,
portable binary encoding, it may still fall short of that goal. Please report
bugs!

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

Strict Aliasing
-

Nocopy uses `reinterpret_cast`, but special care has been taken to avoid strict
aliasing errors. Specifically, `reinterpret_cast` is only used to convert
references to unsigned char to references to POD types whose sole data member is
`std::array<unsigned char, N>`. It is also used to cast scalar types to unsigned
char during endian conversion.

TODO
-

* Callbacks are expected to be passed by value.
