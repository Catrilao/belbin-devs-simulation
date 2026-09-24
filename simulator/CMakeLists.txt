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

find_package(Boost CONFIG REQUIRED)

if(NOT DEFINED CADMIUM_ROOT)
    set(CADMIUM_ROOT $ENV{CADMIUM_ROOT})
endif()

if(NOT CADMIUM_ROOT)
    message(FATAL_ERROR "CADMIUM_ROOT is not defined")
endif()

add_executable(simulation
    src/main.cpp
)

target_include_directories(simulation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(simulation SYSTEM PRIVATE
    ${CADMIUM_ROOT}/include
    ${CADMIUM_ROOT}/DESTimes/include
)

target_link_libraries(simulation PRIVATE Boost::headers)
