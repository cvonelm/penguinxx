// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

#include <catch2/catch_test_macros.hpp>

#include <penguinxx/fd/event_fd.hpp>
#include <penguinxx/fd/mem_fd.hpp>

TEST_CASE("Can open an EventFd", "[can_open_eventfd]")
{
    auto efd_res = penguinxx::EventFd::create();

    REQUIRE(efd_res.ok());

    penguinxx::EventFd fd = efd_res.unpack_ok();

    REQUIRE(fd.write(1234).ok());

    REQUIRE(fd.read().unpack_ok() == 1234);

    REQUIRE(fd == fd);
}

TEST_CASE("Can open EventFd NONBLOCKING", "[can_open_eventfd_nonblocking]")
{
    auto efd_res = penguinxx::EventFd::create(0, { penguinxx::EventFd::Flags::NONBLOCK });

    REQUIRE(efd_res.ok());

    penguinxx::EventFd efd = efd_res.unpack_ok();

    REQUIRE(efd.read().unpack_error().errnum() == bowl::Errno::AGAIN);
}

TEST_CASE("memfd mapping works", "[can_map_memfd]")
{
    auto mem_fd = penguinxx::MemFd::create("bla").unpack_ok();

    REQUIRE(mem_fd.set_size(4096).ok());

    auto mem_map = mem_fd.map(4096).unpack_ok();

    *mem_map.as<uint64_t>() = 42;

    REQUIRE(*mem_map.as<uint64_t>() == 42);

    REQUIRE(mem_map.as<uint64_t[4097]>() == nullptr);
}

TEST_CASE("memfd sealing works", "[can_seal_memfd]")
{
    auto mem_fd =
        penguinxx::MemFd::create("foobar", penguinxx::MemFd::Seal::ALLOW_SEAL).unpack_ok();

    REQUIRE(mem_fd.set_size(4096).ok());

    REQUIRE(mem_fd.seal_grow().ok());

    REQUIRE(!mem_fd.set_size(4097).ok());

    REQUIRE(mem_fd.set_size(4095).ok());
    REQUIRE(mem_fd.seal_shrink().ok());

    REQUIRE(!mem_fd.set_size(4094).ok());

    REQUIRE(mem_fd.seal_write().ok());

    REQUIRE(!penguinxx::MemoryMap::create(mem_fd.to_weak(), 4094, PROT_READ | PROT_WRITE).ok());
}

TEST_CASE("memfd sealing sealing works", "[can_seal_seal_memfd]")
{
    auto mem_fd =
        penguinxx::MemFd::create("foobar", penguinxx::MemFd::Seal::ALLOW_SEAL).unpack_ok();

    REQUIRE(mem_fd.set_size(4096).ok());

    REQUIRE(mem_fd.seal_sealing().ok());

    REQUIRE(!mem_fd.seal_grow().ok());
}
