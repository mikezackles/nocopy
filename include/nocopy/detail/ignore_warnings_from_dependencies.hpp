#ifndef UUID_A8E97FE2_C82D_4689_A457_F3152C598FB2
#define UUID_A8E97FE2_C82D_4689_A457_F3152C598FB2

#ifdef __clang__
  #define BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wdocumentation-unknown-command\"") \
    _Pragma("clang diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("clang diagnostic ignored \"-Wdocumentation\"")
    _Pragma("clang diagnostic ignored \"-Wshadow\"") \
    _Pragma("clang diagnostic ignored \"-Wdouble-promotion\"") \
    _Pragma("clang diagnostic ignored \"-Wclass-varargs\"") \
    _Pragma("clang diagnostic ignored \"-Wexit-time-destructors\"") \
    _Pragma("clang diagnostic ignored \"-Wnewline-eof\"")
#else
  #define BEGIN_IGNORE_WARNINGS_FROM_DEPENDENCIES
#endif

#ifdef __clang__
  #define END_IGNORE_WARNINGS_FROM_DEPENDENCIES \
    _Pragma("clang diagnostic pop")
#else
  #define END_IGNORE_WARNINGS_FROM_DEPENDENCIES
#endif

#endif
