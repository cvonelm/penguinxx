// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

#include <penguinxx/cpu.hpp>
#include <penguinxx/numa.hpp>
#include <penguinxx/topology.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Cpu::current() works")
{
    auto res = penguinxx::Cpu::current();

    REQUIRE(res.ok());
}

TEST_CASE("Cpu::frequency() works")
{
    auto res = penguinxx::Cpu::current().unpack_ok().frequency();

    REQUIRE(res.ok());
}

TEST_CASE("Cpu::governor() works")
{
    auto res = penguinxx::Cpu::current().unpack_ok().governor();

    REQUIRE(res.ok());
}

TEST_CASE("Cpu::available_governors() works")
{
    auto res = penguinxx::Cpu::current().unpack_ok().available_governors();

    REQUIRE(res.ok());
    REQUIRE(res.unpack_ok().size() != 0);
}

TEST_CASE("Cpu::available_frequencies() works")
{
    std::ifstream ifs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_driver");

    std::string driver;
    ifs >> driver;
    if (driver == "acpi-cpufreq")
    {
        REQUIRE(penguinxx::Cpu::current().unpack_ok().available_frequencies().ok());
    }
    else if (driver == "cppc_cpufreq")
    {
        REQUIRE(penguinxx::Cpu::current().unpack_ok().available_frequencies().ok());
    }
    else
    {
        REQUIRE(!penguinxx::Cpu::current().unpack_ok().available_frequencies().ok());
    }
}

TEST_CASE("CpuTopology works")
{
    penguinxx::CpuTopology::instance().cpus();
}

TEST_CASE("Cpu::turbo() works")
{
    REQUIRE(penguinxx::Cpu::turbo().ok());
}

TEST_CASE("NUMANodeSet works")
{
    REQUIRE(penguinxx::NUMANodeSet::of_cpu(penguinxx::Cpu::current().unpack_ok()).ok());
}

TEST_CASE("penguinxx::CpuSet::to_str")
{
    auto res = penguinxx::CpuSet::from_range_str("1,2,3,4");
    auto cpus = res.unpack_ok();

    REQUIRE(cpus.to_str() == "1,2,3,4");
}
