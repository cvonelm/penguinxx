// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/pthread/barrier.hpp>
#include <penguinxx/pthread/thread.hpp>
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
    *(uint64_t*)arg = 42;

    return nullptr;
}

TEST_CASE("penguinxx::Pthread")
{
    uint64_t foo = 0;
    auto res = penguinxx::Pthread::create(test_func, &foo);

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
