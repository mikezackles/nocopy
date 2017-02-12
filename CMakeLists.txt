if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

  set(DEPENDENCY_CMAKE_TOOLS_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bundle/cmake_tools" CACHE PATH
    "the location of the cmake_tools helper repository")
  list(APPEND CMAKE_MODULE_PATH "${DEPENDENCY_CMAKE_TOOLS_LOCATION}")

  set(DEPENDENCY_HANA_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bundle/hana" CACHE PATH
    "the location of the hana library")
  set(DEPENDENCY_GSL_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bundle/gsl" CACHE PATH
    "the location of the GSL library")
  set(DEPENDENCY_SILENCE_DEPENDENCIES_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bundle/silence_dependencies" CACHE PATH
    "the location of the silence_dependencies helper")
  set(DEPENDENCY_LAMBDA_OVERLOAD_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bundle/lambda_overload" CACHE PATH
    "the location of the lambda_overload helper")
  set(DEPENDENCY_CATCH_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bundle/catch" CACHE PATH
    "the location of the catch testing library")

  set(OPTIMIZE_LITTLE_ENDIAN false CACHE BOOL
    "optimize for little-endian architectures (NOT FOR USE ON BIG-ENDIAN PLATFORMS)")

  include("default_build_type")
  default_build_type(Release)
  project(nocopy VERSION 0.1 LANGUAGES CXX)

  enable_testing()
  set(CMAKE_CTEST_COMMAND ctest -V)
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

  add_subdirectory("${DEPENDENCY_LAMBDA_OVERLOAD_LOCATION}" EXCLUDE_FROM_ALL)
endif()

add_library(nocopy INTERFACE)
target_compile_definitions(nocopy
  INTERFACE
  $<$<BOOL:${OPTIMIZE_LITTLE_ENDIAN}>:NOCOPY_OPTIMIZE_LITTLE_ENDIAN>)
target_include_directories(nocopy
  INTERFACE
  "include"
  "${DEPENDENCY_HANA_LOCATION}/include"
  "${DEPENDENCY_GSL_LOCATION}/include"
  "${DEPENDENCY_SILENCE_DEPENDENCIES_LOCATION}/include")
target_link_libraries(nocopy INTERFACE lambda_overload)
# TODO - whitelist features here if necessary
include("target_use_modern_cxx")
target_use_modern_cxx(nocopy TYPE INTERFACE)

add_executable(nocopy_tests EXCLUDE_FROM_ALL
  "test/main.cpp"
  "test/schema.cpp"
  "test/structpack.cpp"
  "test/oneof.cpp"
  "test/heap.cpp")

target_include_directories(nocopy_tests
  PRIVATE "${DEPENDENCY_CATCH_LOCATION}/include")

include("target_all_warnings_except")
target_all_warnings_except(nocopy_tests
  CLANG
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-weak-vtables # AFAIK there's not much to do about this one while remaining header-only
  -Wno-padded
  GCC
  -Wno-unknown-pragmas # catch
  )

include("target_add_sanitizers")
target_add_sanitizers(nocopy_tests)

target_link_libraries(nocopy_tests PRIVATE nocopy)

enable_testing()
add_test(NAME nocopy_tests COMMAND nocopy_tests)

if(TARGET check)
  add_dependencies(check nocopy_tests)
endif()