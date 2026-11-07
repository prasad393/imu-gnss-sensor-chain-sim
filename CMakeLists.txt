cmake_minimum_required(VERSION 3.14)
project(sensor_chain_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_library(simcore STATIC
  src/component.cpp
  src/sensors.cpp
  src/processing.cpp
  src/fdir.cpp
  src/logger.cpp
  src/scenarios.cpp)
target_include_directories(simcore PUBLIC src)
target_compile_options(simcore PUBLIC -Wall -Wextra -Wpedantic)
target_link_libraries(simcore PUBLIC Threads::Threads)

add_executable(sim src/main.cpp)
target_link_libraries(sim PRIVATE simcore)

enable_testing()
add_executable(test_rules tests/test_rules.cpp)
target_link_libraries(test_rules PRIVATE simcore)
add_test(NAME rules COMMAND test_rules)
