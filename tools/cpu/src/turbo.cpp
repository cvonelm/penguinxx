// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/cpu.hpp>
#include <penguinxx/topology.hpp>

#include <iostream>

static void print_turbo_help()
{
    std::cerr << "penguinxx-cpu turbo [subcommand] \n";
    std::cerr << "\n";
    std::cerr << "Available subcommands:\n";
    std::cerr << "\t- get -- Get current turbo state\n";
    std::cerr << "\t- set [ON/OFF] -- Set turbo state\n";
    std::cerr << "\t- help -- Print this help\n";
}

void parse_turbo(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "penguinxx-cpu turbo requires a subcommand\n";
        std::exit(1);
    }

    if (std::string("get") == argv[1])
    {
        auto turbo = penguinxx::Cpu::current().unpack_ok().turbo().unpack_ok();

        if (turbo == penguinxx::TurboState::ON)
        {
            std::cout << "ON\n";
        }
        else
        {
            std::cout << "OFF\n";
        }
    }
    else if (std::string("set") == argv[1])
    {
        if (argc < 3)
        {
            std::cerr << "penguinxx-cpu turbo set requires an argument!\n";
            std::exit(1);
        }
        auto ts = penguinxx::TurboState::ON;
        if (std::string("ON") == argv[2])
        {
            ts = penguinxx::TurboState::ON;
        }
        else if (std::string("OFF") == argv[2])
        {
            ts = penguinxx::TurboState::OFF;
        }
        else
        {
            std::cerr << "Unknown turbo setting: " << argv[2] << "\n";
        }

        for (auto& cpu : penguinxx::CpuTopology::instance().cpus())
        {
            auto res = cpu.turbo(ts);
            if (!res.ok())
            {
                std::cerr << "Could not set turbo state for Cpu " << cpu.as_int() << ": "
                          << res.unpack_error().display() << std::endl;
                std::exit(1);
            }
        }
    }
    else if (std::string("help") == argv[1])
    {
        print_turbo_help();
        std::exit(0);
    }
    else
    {
        std::cerr << "Unknown penguinxx-cpu turbo command \"" << argv[1] << "\"\n";
        std::exit(1);
    }
}
