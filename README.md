* Inspired by FlatBuffers, but not compatible
* Support fixed-length binary data
* JSON output for debugging (input too?)
* Build moving forward in memory
  * Simplifies bookkeeping and allows for reusing things like std::vector
* No structs
  * simplifies implementation
* Modern C++-only
  * simplifies implementation
  * Use the compiler for code generation
    * Makes generic code generation possible
    * Eliminates extra build step
    
This project is highly experimental, and although it aims to provide a flexible,
portable binary encoding, it may still fall short of that goal. Please report
bugs!
