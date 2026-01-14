# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

cmake_minimum_required(VERSION 3.11)
project(penguinxx VERSION 0.0.1)

include(FeatureSummary)

if(${PROJECT_IS_TOP_LEVEL})
    option(PENGUINXX_TESTS "Build penguinxx tests" ON)
    option(PENGUINXX_TOOLS "Build penguinxx tools" ON)
else()
    option(PENGUINXX_TESTS "Build penguinxx tests" OFF)
    option(PENGUINXX_TOOLS "Build penguinxx tools" OFF)
endif()

add_feature_info("PENGUINXX_TESTS" PENGUINXX_TESTS "Build penguinxx tests")
add_feature_info("PENGUINXX_TOOLS" PENGUINXX_TOOLS "Build penguinxx tools")

add_subdirectory(lib/bowl)

find_package(fmt REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NUMA REQUIRED IMPORTED_TARGET numa)

add_library(penguinxx-core src/util.cpp)
target_include_directories(penguinxx-core PUBLIC include)
target_link_libraries(penguinxx-core Bowl::bowl fmt::fmt)
add_library(Penguinxx::core ALIAS penguinxx-core)

add_library(libpenguinxx-cpu src/cpu.cpp)
target_include_directories(libpenguinxx-cpu PUBLIC include)
target_link_libraries(libpenguinxx-cpu PUBLIC Penguinxx::core Bowl::bowl PkgConfig::NUMA)
add_library(Penguinxx::cpu ALIAS libpenguinxx-cpu)

if(${PENGUINXX_TESTS})
    find_package(Catch2 REQUIRED)
    include(CTest)
    include(Catch)

    add_executable(tests_core tests/tests_core.cpp)
    target_link_libraries(tests_core PRIVATE Catch2::Catch2WithMain Penguinxx::core)
    catch_discover_tests(tests_core)

    add_executable(tests_cpu tests/tests_cpu.cpp)
    target_link_libraries(tests_cpu PRIVATE Catch2::Catch2WithMain Penguinxx::cpu)
    catch_discover_tests(tests_cpu)
endif()

if(${PENGUINXX_TOOLS})
    add_subdirectory(tools)
endif()

feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES)
