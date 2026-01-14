// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include "governor.hpp"

#include <penguinxx/cpu.hpp>
#include <penguinxx/topology.hpp>

#include <iostream>

void print_governor_help()
{
    std::cerr << "penguinxx-cpu governor [subcommand]\n";
    std::cerr << "\n";
    std::cerr << "Available subcommands:\n";
    std::cerr << "\t - get -- Get current governor\n";
    std::cerr << "\t - set -- Set governor\n";
    std::cerr << "\t - help -- print this help\n";
}

void parse_governor(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "penguinxx-cpu governor needs a subcommand!\n\n";
        std::exit(1);
    }

    if (std::string("get") == argv[1])
    {
        std::cerr << penguinxx::governor_to_string(
                         penguinxx::Cpu::current().unpack_ok().governor().unpack_ok())
                  << std::endl;
    }
    else if (std::string("set") == argv[1])
    {
        if (argc < 3)
        {
            std::cerr << "penguinxx-cpu governor set requires the name of a governor!\n";
            std::exit(1);
        }

        auto gov_res = penguinxx::string_to_governor(argv[2]);

        if (!gov_res.ok())
        {
            std::cerr << "Could not parse governor: " << gov_res.unpack_error().display()
                      << std::endl;
            std::exit(1);
        }

        auto gov = gov_res.unpack_ok();

        for (auto cpu : penguinxx::CpuTopology::instance().cpus())
        {
            auto res = cpu.governor(gov);
            if (!res.ok())
            {
                std::cerr << "Could not set governor for cpu " << cpu.as_int() << ": "
                          << res.unpack_error().display() << std::endl;
                std::exit(1);
            }
        }
    }
    else if (std::string("help") == argv[1])
    {
        print_governor_help();
        std::exit(0);
    }
    else
    {
        std::cerr << "Unknown subcommand " << argv[1] << std::endl;
        std::exit(1);
    }
    std::exit(0);
}
