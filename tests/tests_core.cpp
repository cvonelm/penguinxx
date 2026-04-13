// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/clock.hpp>
#include <penguinxx/pthread/barrier.hpp>
#include <penguinxx/pthread/thread.hpp>
#include <penguinxx/syscalls.hpp>
#include <penguinxx/util.hpp>

#include <bowl/exception.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("read_from_file_list() works")
{
    std::fstream fs("foobar", std::fstream::out | std::fstream::trunc);
    std::string teststr = "foo bar baz";

    fs << teststr;

    fs.close();

    auto res = penguinxx::read_from_file_list<std::string>("foobar");

    REQUIRE(res.ok());

    auto vec = res.unpack_ok();

    REQUIRE(vec.size() == 3);

    REQUIRE(vec[0] == "foo");
    REQUIRE(vec[1] == "bar");
    REQUIRE(vec[2] == "baz");

    REQUIRE(!penguinxx::read_from_file_list<uint64_t>("foobar").ok());
    REQUIRE(!penguinxx::read_from_file_list<uint64_t>("veryveryveryfakefile").ok());
}

TEST_CASE("set_from_range_str")
{
    REQUIRE(penguinxx::set_from_range_str("0").ok());
    REQUIRE(!penguinxx::set_from_range_str("0foobar").ok());
    REQUIRE(!penguinxx::set_from_range_str("0foobar-foobar").ok());
    REQUIRE(penguinxx::set_from_range_str("0-12,14,240").ok());

    REQUIRE(!penguinxx::set_from_range_str("14-10").ok());

    auto res = penguinxx::set_from_range_str("12,14,100-103,42-46");

    REQUIRE(res.ok());

    REQUIRE(res.unpack_ok().size() == 11);
}

void* test_func(void* arg)
{
    *reinterpret_cast<uint64_t*>(arg) = 42; // NOLINT (cppcoreguidelines-avoid-magic-numbers)

    return nullptr;
}

int test_val = 0; // NOLINT (cppcoreguidelines-avoid-non-const-global-variables)

void happy_lil_func(int boof)
{
    test_val = boof;
}

TEST_CASE("penguinxx::Pthread::create")
{
    uint64_t foo = 42; // NOLINT (cppcoreguidelines-avoid-magic-numbers)
    auto res = penguinxx::Pthread::create(happy_lil_func, foo);

    REQUIRE(res.ok());
    auto pthread = res.unpack_ok();

    pthread.join();

    REQUIRE(test_val == 42); // NOLINT (cppcoreguidelines-avoid-magic-numbers)
}

TEST_CASE("penguinxx::Pthread::crate_raw")
{
    uint64_t foo = 0;
    auto res = penguinxx::Pthread::create_raw(test_func, &foo);

    REQUIRE(res.ok());
    auto pthread = res.unpack_ok();

    pthread.join();

    REQUIRE(foo == 42);
}

TEST_CASE("penguinxx::Barrier")
{
    auto res = penguinxx::Barrier::create(1);

    REQUIRE(res.ok());

    auto barrier = res.unpack_ok();

    REQUIRE(barrier.wait().ok());
}

TEST_CASE("penguinxx::Clock")
{
    REQUIRE(penguinxx::Clock::gettime(penguinxx::Clocks::REALTIME).ok());

    auto tp1 = penguinxx::Clock::gettime(penguinxx::Clocks::MONOTONIC_RAW).unpack_ok();
    auto tp2 = penguinxx::Clock::gettime(penguinxx::Clocks::MONOTONIC_RAW).unpack_ok();

    REQUIRE(tp2 > tp1);
}

TEST_CASE("Syscalls::readlink works")
{
    REQUIRE(penguinxx::Syscalls::readlink("/proc/self").ok());
}

TEST_CASE("Syscalls::lstat works")
{
    REQUIRE(penguinxx::Syscalls::lstat("/proc/self").ok());
}
