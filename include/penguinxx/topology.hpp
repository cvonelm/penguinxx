// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/macros.hpp>

#include <penguinxx/cpu.hpp>
#include <penguinxx/cpu_set.hpp>

#include <string>

namespace penguinxx
{

class CpuTopology
{
public:
    /*
     * Returns an instance of the CpuTopology
     */
    static CpuTopology& instance()
    {
        static CpuTopology t = CpuTopology::create().unpack_ok();

        return t;
    }

    /*
     * Returns a CpuSet of the CPUs in the current system.
     */
    [[nodiscard]] CpuSet cpus() const
    {
        return cores_;
    }

private:
    static bowl::Expected<CpuTopology, bowl::CustomError> create()
    {
        CpuTopology topo;

        CHECK_ASSIGN(online_list, read_from_file<std::string>("/sys/devices/system/cpu/online"));

        CHECK_ASSIGN(online, set_from_range_str(online_list));

        // We use Cpu(int cpu) instead of Cpu::from_int(int cpu) here,
        // because the latter is checked against this topology for validity.
        // This would result in recursive initialization errors and is useless,
        // because we know that the ints coming in here are valid cpuids.
        for (auto cpu : online)
        {
            topo.cores_.add(Cpu(cpu));
        }

        return topo;
    }

    CpuSet cores_;
};
} // namespace penguinxx
