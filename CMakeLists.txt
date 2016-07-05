cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(nocopy VERSION 0.1 LANGUAGES CXX)

include("${CMAKE_SOURCE_DIR}/cmake/target_all_warnings_except.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/target_use_modern_cxx.cmake")

set(HANA_LOCATION "${CMAKE_SOURCE_DIR}/bundle/hana" CACHE PATH
  "the location of the hana library")
set(OPTIONAL_LOCATION "${CMAKE_SOURCE_DIR}/bundle/optional" CACHE PATH
  "the location of the std::experimental::optional implementation")

set(CMAKE_CXX_EXTENSIONS OFF) # Turn off gnu extensions

add_library(nocopy INTERFACE)
target_compile_definitions(nocopy
  INTERFACE
  ASIO_STANDALONE)
target_include_directories(nocopy
  INTERFACE
  "include"
  "${HANA_LOCATION}/include"
  "${OPTIONAL_LOCATION}")
#target_link_libraries(nocopy INTERFACE sodium ${CMAKE_THREAD_LIBS_INIT})
# TODO - whitelist features here if necessary
target_use_modern_cxx(nocopy TYPE INTERFACE)

set(test_sources
  "test/main.cpp"
  "test/archive.cpp"
  "test/datapack.cpp")

add_executable(little_endian_tests ${test_sources})
add_executable(big_endian_tests ${test_sources})

target_include_directories(little_endian_tests
  PRIVATE
  "${CMAKE_SOURCE_DIR}/bundle/catch/include")

target_include_directories(big_endian_tests
  PRIVATE
  "${CMAKE_SOURCE_DIR}/bundle/catch/include")

target_compile_definitions(big_endian_tests
  PRIVATE
  NOCOPY_LITTLEENDIAN=0)

set(shared_warnings
  CLANG
  # clang warnings
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  GCC
  # gcc warnings
  -Wno-unknown-pragmas # catch
  )
target_all_warnings_except(little_endian_tests ${shared_warnings})
target_all_warnings_except(big_endian_tests ${shared_warnings})

target_link_libraries(little_endian_tests nocopy)
target_link_libraries(big_endian_tests nocopy)

enable_testing()
add_test(little_endian little_endian_tests)
add_test(big_endian big_endian_tests)
