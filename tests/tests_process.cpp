// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

#include <penguinxx/process.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Process::me() works")
{
    REQUIRE(penguinxx::Process::me().as_pid_t() == getpid());
}

TEST_CASE("Process.comm() works")
{
    REQUIRE(penguinxx::Process::me().comm().ok());
}
