# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

add_executable(penguinxx-cpu src/main.cpp src/cstate.cpp src/frequency.cpp src/governor.cpp src/turbo.cpp)
target_include_directories(penguinxx-cpu PRIVATE include)
target_link_libraries(penguinxx-cpu PRIVATE penguinxx::cpu)

install(
    TARGETS penguinxx-cpu
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
