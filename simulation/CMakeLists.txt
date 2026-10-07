cmake_minimum_required(VERSION 3.16)
project(wayfarer-sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(wayfarer_sim
  src/Commodity.cpp
  src/Market.cpp
  src/ShipHull.cpp
  src/CargoHold.cpp
  src/Faction.cpp
  src/WorldLocation.cpp
  src/WorldSimulation.cpp
  src/DataLoader.cpp
  src/PioneerWorldAdapter.cpp
)

target_include_directories(wayfarer_sim
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_compile_options(wayfarer_sim PRIVATE -Wall -Wextra -Wpedantic)

# Standalone build: tests + headless Stage 0 demo.
# When embedded into Pioneer via add_subdirectory, skip these targets.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  enable_testing()

  add_executable(wayfarer_sim_tests
    tests/test_main.cpp
    tests/test_market.cpp
    tests/test_cargo.cpp
    tests/test_trade.cpp
    tests/test_contract.cpp
    tests/test_determinism.cpp
  )
  target_link_libraries(wayfarer_sim_tests PRIVATE wayfarer_sim)
  add_test(NAME wayfarer_sim_tests COMMAND wayfarer_sim_tests)

  add_executable(wayfarer_stage0_demo demo/stage0_demo.cpp)
  target_link_libraries(wayfarer_stage0_demo PRIVATE wayfarer_sim)
  target_compile_definitions(wayfarer_stage0_demo PRIVATE
    WAYFARER_DATA_DIR="${CMAKE_SOURCE_DIR}/../data/wayfarer"
  )
endif()
