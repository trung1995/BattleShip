cmake_minimum_required(VERSION 3.16)
project(BattleShip LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BATTLESHIP_BUILD_GAME "Build the cocos2d-x game executable" ON)
option(BATTLESHIP_BUILD_TESTS "Build the unit and simulation tests" ON)

# Project scripts are included by path; cmake/GoogleTest.cmake would shadow CMake's own module.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Warnings.cmake")

# Rules core and application logic: standard library only, never the engine (Principle I).
add_subdirectory(src/core)
add_subdirectory(src/app)

if(BATTLESHIP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BATTLESHIP_BUILD_GAME)
    add_subdirectory(src/game)
endif()
