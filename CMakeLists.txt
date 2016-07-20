cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include("target_add_sanitizers")
include("target_all_warnings_except")
include("target_use_modern_cxx")
include("default_build_type")

default_build_type(Release)
project(nocopy VERSION 0.1 LANGUAGES CXX)

set(HANA_LOCATION "${CMAKE_SOURCE_DIR}/bundle/hana" CACHE PATH
  "the location of the hana library")
set(OPTIMIZE_LITTLE_ENDIAN false CACHE BOOL "optimize for little-endian architectures (NOT FOR USE ON BIG-ENDIAN PLATFORMS)")

set(CMAKE_CXX_EXTENSIONS OFF) # Turn off gnu extensions

add_library(nocopy INTERFACE)
target_compile_definitions(nocopy
  INTERFACE
  ASIO_STANDALONE
  $<$<BOOL:${OPTIMIZE_LITTLE_ENDIAN}>:NOCOPY_OPTIMIZE_LITTLE_ENDIAN>)
target_include_directories(nocopy
  INTERFACE
  "include"
  "${HANA_LOCATION}/include")
#target_link_libraries(nocopy INTERFACE sodium ${CMAKE_THREAD_LIBS_INIT})
# TODO - whitelist features here if necessary
target_use_modern_cxx(nocopy TYPE INTERFACE)

add_executable(tests
  "test/main.cpp"
  "test/archive.cpp"
  "test/datapack.cpp"
  "test/oneof.cpp"
  "test/raw_heap.cpp")

target_include_directories(tests
  PRIVATE
  "${CMAKE_SOURCE_DIR}/bundle/catch/include")

target_all_warnings_except(tests
  CLANG
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-weak-vtables # AFAIK there's not much to do about this one while remaining header-only
  -Wno-padded
  GCC
  -Wno-unknown-pragmas # catch
  )

target_add_sanitizers(tests)

target_link_libraries(tests PRIVATE nocopy)

enable_testing()
add_test(tests tests)
