# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

add_executable(penguinxx-cpu src/main.cpp src/frequency.cpp src/governor.cpp src/turbo.cpp)
target_include_directories(penguinxx-cpu PRIVATE include)
target_link_libraries(penguinxx-cpu PRIVATE Penguinxx::cpu)
