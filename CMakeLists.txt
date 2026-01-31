# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

cmake_minimum_required(VERSION 3.11)
project(penguinxx VERSION 0.0.1)

include(CMakePackageConfigHelpers)
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

add_library(core src/util.cpp)
target_include_directories(core PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")
target_link_libraries(core Bowl::bowl fmt::fmt)
set_target_properties(core PROPERTIES PREFIX "libpenguinxx-")
add_library(penguinxx::core ALIAS core)

add_library(cpu src/cpu.cpp)
target_include_directories(cpu PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")
target_link_libraries(cpu PUBLIC penguinxx::core Bowl::bowl PkgConfig::NUMA)
set_target_properties(cpu PROPERTIES PREFIX "libpenguinxx-")
add_library(penguinxx::cpu ALIAS cpu)

if(${PENGUINXX_TESTS})
    find_package(Catch2 REQUIRED)
    include(CTest)
    include(Catch)

    add_executable(tests_core tests/tests_core.cpp)
    target_link_libraries(tests_core PRIVATE Catch2::Catch2WithMain penguinxx::core)
    catch_discover_tests(tests_core)

    add_executable(tests_cpu tests/tests_cpu.cpp)
    target_link_libraries(tests_cpu PRIVATE Catch2::Catch2WithMain penguinxx::cpu)
    catch_discover_tests(tests_cpu)

    add_executable(tests_process tests/tests_process.cpp)
    target_link_libraries(tests_process PRIVATE Catch2::Catch2WithMain penguinxx::core)
    catch_discover_tests(tests_process)
endif()

if(${PENGUINXX_TOOLS})
    add_subdirectory(tools)
endif()

include(GNUInstallDirs)

set(project_config "${CMAKE_CURRENT_BINARY_DIR}/penguinxx-config.cmake")

set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/penguinxx")
configure_package_config_file(
    "cmake/penguinxx-config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)
install(
    DIRECTORY "include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    TARGETS core cpu bowl
    EXPORT "penguinxx-targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    FILES "${project_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT "penguinxx-targets"
    NAMESPACE "penguinxx::"
    DESTINATION "${config_install_dir}"
)
feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES)
