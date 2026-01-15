// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/cpu.hpp>
#include <penguinxx/topology.hpp>

#include <iostream>

static void print_cstate_help()
{
    std::cerr << "penguinxx-cpu cstate [subcommand] \n";
    std::cerr << "\n";
    std::cerr << "Available subcommands:\n";
    std::cerr << "\t- get -- Get enabled/disabled status of cstates\n";
    std::cerr << "\t- list -- List cstates of this system, one per line\n";
    std::cerr << "\t- set [ON/OFF] -- Set cstate state\n";
    std::cerr << "\t- help -- Print this help\n";
}

void parse_cstate(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "penguinxx-cpu cstate requires a subcommand\n";
        std::exit(1);
    }

    if (std::string("get") == argv[1])
    {
        auto cstates = penguinxx::Cpu::current().unpack_ok().get_cstates().unpack_ok();

        for (auto cstate : cstates)
        {
            if (cstate.second == penguinxx::CStateState::ENABLED)
            {
                std::cout << cstate.first.get_name() << ": ENABLED" << std::endl;
            }
            else
            {
                std::cout << cstate.first.get_name() << ": DISABLED" << std::endl;
            }
        }

        exit(0);
    }
    else if (std::string("enable") == argv[1])
    {
        if (argc < 3)
        {
            std::cerr << "penguinxx-cpu cstate enable requires the name of a cstate!" << std::endl;
            std::exit(1);
        }
        for (auto cpu : penguinxx::CpuTopology::instance().cpus())
        {
            auto cstate_res = cpu.cstate_from_str(argv[2]);

            if (!cstate_res.ok())
            {
                std::cout << cstate_res.unpack_error().display() << std::endl;
                std::exit(1);
            }
            auto cstate = cstate_res.unpack_ok();

            auto res = cpu.set_cstate(cstate, penguinxx::CStateState::ENABLED);

            if (!res.ok())
            {
                std::cerr << "Could not set cstate for CPU " << cpu.as_int() << ": "
                          << res.unpack_error().display() << std::endl;
                std::exit(1);
            }
        }
    }
    else if (std::string("list") == argv[1])
    {
        auto cstates = penguinxx::Cpu::current().unpack_ok().get_cstates().unpack_ok();

        for (auto cstate : cstates)
        {
            std::cout << cstate.first.get_name() << std::endl;
        }
    }
    else if (std::string("help") == argv[1])
    {
        print_cstate_help();
        std::exit(0);
    }
    else
    {
        std::cerr << "Unknown penguinxx-cpu cstate command \"" << argv[1] << "\"\n";
        std::exit(1);
    }
}
