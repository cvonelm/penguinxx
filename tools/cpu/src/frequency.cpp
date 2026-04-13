// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include "penguinxx/clock.hpp"
#include <ctime>
#include <penguinxx/cpu.hpp>
#include <penguinxx/topology.hpp>

#include <iostream>

static void print_frequency_help()
{
    std::cerr << "penguinxx-cpu frequency [subcommand]\n";
    std::cerr << "\n";
    std::cerr << "Available subcommands:\n";
    std::cerr << "\t - get -- Get current frequency\n";
    std::cerr << "\t - set [FREQ_IN_GHZ] -- Sets frequency to FREQ_IN_GHZ. Required \"userspace\" "
                 "governor.\n";
    std::cerr << "\t - list -- Lists available frequencies\n";
    std::cerr << "\t - help -- Print this help\n";
}

void parse_frequency(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "penguinxx-cpu frequency needs the name of a subcommand!\n";
        print_frequency_help();
        exit(1);
    }

    if (std::string("help") == argv[1])
    {
        print_frequency_help();
        exit(0);
    }
    else if (std::string("list") == argv[1])
    {
        auto cpu = penguinxx::Cpu::current().unpack_ok();

        auto avail_freq_res = cpu.available_frequencies();

        if (!avail_freq_res.ok())
        {
            std::cerr << "Could not list available frequencies: "
                      << avail_freq_res.unpack_error().display() << std::endl;
            exit(1);
        }

        auto avail_freq = avail_freq_res.unpack_ok();
        for (const auto freq : avail_freq)
        {
            std::cout << freq << std::endl;
        }
    }
    else if (std::string("set") == argv[1])
    {
        if (penguinxx::Cpu::current().unpack_ok().governor().unpack_ok() !=
            penguinxx::Governor::USERSPACE)
        {
            std::cerr << "Frequency can only be set if the governor is set to \"userspace\"!\n";
            std::exit(1);
        }

        if (argc < 3)
        {
            std::cerr << "penguinxx-cpu frequency set requires a FREQUENCY argument!\n";
            print_frequency_help();
            std::exit(1);
        }

        for (auto core : penguinxx::CpuTopology::instance().cpus())
        {
            auto a = penguinxx::Clock::gettime(penguinxx::Clocks::MONOTONIC_RAW).unpack_ok();
            auto res = core.frequency(std::stoull(argv[2]));
            auto b = penguinxx::Clock::gettime(penguinxx::Clocks::MONOTONIC_RAW).unpack_ok();

            std::cerr << b - a;
            if (!res.ok())
            {
                std::cerr << "Could not set frequency for cpu " << core.as_int() << ": "
                          << res.unpack_error().display() << std::endl;
                exit(1);
            }
        }
    }

    else if (std::string("get") == argv[1])
    {
        std::cout << penguinxx::Cpu::current().unpack_ok().frequency().unpack_ok();
        std::exit(0);
    }

    else
    {
        std::cerr << "Unknown penguinxx-cpu frequency command \"" << argv[1] << "!\n";
        std::exit(1);
    }
}
