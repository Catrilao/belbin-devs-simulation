cmake_minimum_required(VERSION 3.15)
project(Belbin_Simulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

if(DEFINED ENV{IN_NIX_SHELL})
    set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "")
endif()

if(NOT DEFINED CADMIUM_ROOT)
    set(CADMIUM_ROOT $ENV{CADMIUM_ROOT})
endif()

if(NOT CADMIUM_ROOT)
    message(FATAL_ERROR "CADMIUM_ROOT is not defined")
endif()

add_library(belbin_core INTERFACE)

target_include_directories(belbin_core INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(belbin_core SYSTEM INTERFACE
  ${CADMIUM_ROOT}/include
  ${CADMIUM_ROOT}/json/include
)

add_executable(simulation src/main.cpp)
target_link_libraries(simulation PRIVATE belbin_core)
target_compile_options(simulation PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  add_executable(belbin_tests tests/test_simulator.cpp)
  target_link_libraries(belbin_tests PRIVATE belbin_core GTest::gtest_main)
  target_compile_options(belbin_tests PRIVATE -Wall -Wextra -Wpedantic)
  include(GoogleTest)
  gtest_discover_tests(belbin_tests)
endif()
