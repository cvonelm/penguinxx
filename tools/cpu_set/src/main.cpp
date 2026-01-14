// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/cpu_set.hpp>
#include <penguinxx/topology.hpp>

#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Missing argument pattern" << std::endl;
        std::cerr << "penguinxx-cpu-set [PATTERN_NAME]" << std::endl;
        std::cerr << "\t- staircase" << std::endl;
        std::cerr << "\t- all" << std::endl;
        return 1;
    }
    penguinxx::CpuSet set;

    if (strcmp(argv[1], "staircase") == 0)
    {
        for (const auto& cpu : penguinxx::CpuTopology::instance().cpus())
        {
            set.add(cpu);
            std::cout << set.to_str() << std::endl;
        }
    }
    else if (strcmp(argv[1], "all") == 0)
    {
        std::cout << penguinxx::CpuTopology::instance().cpus().to_str();
    }
    else
    {
        std::cerr << "Unknown CPU pattern: " << argv[1] << std::endl;
        return 1;
    }

    return 0;
}
