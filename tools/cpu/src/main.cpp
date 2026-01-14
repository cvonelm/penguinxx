// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include "frequency.hpp"
#include "governor.hpp"
#include "turbo.hpp"

#include <iostream>

static void print_help()
{
    std::cerr << "penguinxx-cpu [subcommand]\n";
    std::cerr << "\n";
    std::cerr << "Available Subcommands: \n";
    std::cerr << "\t- frequency\n";
    std::cerr << "\t- governor\n";
    std::cerr << "\t- turbo\n";
    std::cerr << "\t- help\n";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        print_help();
        exit(1);
    }
    if (std::string("frequency") == argv[1])
    {
        parse_frequency(argc - 1, argv + 1);
    }
    else if (std::string("governor") == argv[1])
    {
        parse_governor(argc - 1, argv + 1);
    }
    else if (std::string("turbo") == argv[1])
    {
        parse_turbo(argc - 1, argv + 1);
    }
    else if (std::string("help") == argv[1])
    {
        print_help();
    }
    else
    {
        std::cerr << "Unknown subcommand: " << argv[1] << std::endl;
    }

    return 0;
}
