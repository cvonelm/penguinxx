// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

#include <penguinxx/process.hpp>
#include <penguinxx/thread.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iostream>

TEST_CASE("Process::me() works")
{
    REQUIRE(penguinxx::Process::me().as_pid_t() == getpid());
}

TEST_CASE("Process.comm() works")
{
    REQUIRE(penguinxx::Process::me().comm().ok());
}

TEST_CASE("Process.exe() works")
{
    REQUIRE(penguinxx::Process::me().exe().ok());

    REQUIRE(penguinxx::Process::me().exe().unpack_ok().filename() == "tests_process");
}

TEST_CASE("Process.cmdline() works")
{
    auto res = penguinxx::Process(1).cmdline();

    REQUIRE(res.ok());

    auto cmdline = res.unpack_ok();
}

TEST_CASE("Thread.comm() works")
{
    REQUIRE(penguinxx::Process::me().as_thread().comm().ok());
}
