# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

add_executable(penguinxx-cpu-set src/main.cpp)
target_link_libraries(penguinxx-cpu-set penguinxx::cpu)

install(
    TARGETS penguinxx-cpu-set
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
