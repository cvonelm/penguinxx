// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/cpu.hpp>
#include <penguinxx/topology.hpp>

#include <algorithm>
#include <iostream>

static void print_cstate_help()
{
    std::cerr << "penguinxx-cpu cstate [subcommand] \n";
    std::cerr << "\n";
    std::cerr << "Available subcommands:\n";
    std::cerr << "\t- get -- Get enabled/disabled status of cstates\n";
    std::cerr << "\t- set [CSTATE],[CSTATE],... enables the comma separated list of cstates. "
                 "Disables all cstates that were not given\n";
    std::cerr << "\t- list -- List all cstates of this system, one per line\n";
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
    else if (std::string("set") == argv[1])
    {
        if (argc < 3)
        {
            std::cerr << "'penguinxx-cpu cstate set' requires the name of a cstate!" << std::endl;
            std::exit(1);
        }

        for (auto cpu : penguinxx::CpuTopology::instance().cpus())
        {
            std::string cstate_str = argv[2];
            std::stringstream ss(cstate_str);

            std::vector<penguinxx::CState> to_enable;
            std::string to_enable_cstate;

            while (std::getline(ss, to_enable_cstate, ','))
            {
                auto parsed_cstate_res = cpu.cstate_from_str(to_enable_cstate);

                if (!parsed_cstate_res.ok())
                {
                    std::cout << "Can not parse " << to_enable_cstate
                              << " as a cstate name: " << parsed_cstate_res.unpack_error().display()
                              << std::endl;
                    exit(1);
                }
                to_enable.emplace_back(parsed_cstate_res.unpack_ok());
            }

            std::vector<penguinxx::CState> to_disable;
            auto all_cstates = cpu.get_cstates().unpack_ok();
            for (auto state : all_cstates)
            {
                if (std::find(to_enable.begin(), to_enable.end(), state.first) == to_enable.end())
                {
                    to_disable.emplace_back(state.first);
                }
            }

            for (auto enable_state : to_enable)
            {
                if (cpu.as_int() == 0)
                {
                    std::cout << "Enabling " << enable_state.get_name() << std::endl;
                }
                auto res = cpu.set_cstate(enable_state, penguinxx::CStateState::ENABLED);

                if (!res.ok())
                {
                    std::cerr << "Could not enable cstate: " << res.unpack_error().display()
                              << std::endl;
                    exit(1);
                }
            }
            for (auto disable_state : to_disable)
            {
                if (cpu.as_int() == 0)
                {
                    std::cout << "Disabling " << disable_state.get_name() << std::endl;
                }
                auto res = cpu.set_cstate(disable_state, penguinxx::CStateState::DISABLED);

                if (!res.ok())
                {
                    std::cerr << "Could not disable cstate: " << res.unpack_error().display()
                              << std::endl;
                    exit(1);
                }
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
    else if (std::string("staircase") == argv[1])
    {
        auto cstates = penguinxx::Cpu::current().unpack_ok().get_cstates().unpack_ok();
        std::vector<std::string> tmp;

        while (!cstates.empty())
        {
            tmp.emplace_back(cstates.front().first.get_name());
            cstates.erase(cstates.begin());

            std::cout << fmt::format("{}", fmt::join(tmp, ",")) << std::endl;
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
