// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/cpu.hpp>
#include <penguinxx/util.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/macros.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <set>

namespace penguinxx
{
class CpuSet
{
public:
    /*
     * Creates a CpuSet, with Cpu(cpuid) from `start` to `end` (end including) in `step` steps.
     *
     * Checks if the generated cpuids are valid cpuids.
     *
     * Returns bowl::CustomError if one of the generated cpuids is not a valid one.
     */
    static bowl::Expected<CpuSet, bowl::CustomError> from_range(uint64_t start, uint64_t end,
                                                                uint64_t step = 1)
    {
        CpuSet res;

        for (std::size_t i = start; i <= end; i += step)
        {
            CHECK_ASSIGN(cpu, Cpu::from_int(i))
            res.cpus_.emplace(cpu);
        }

        return res;
    }

    /*
     * Creates a CpuSet from a range string `range_str`. This is a string of the format:
     * "1,3,10-15,20"
     *
     * Fails with bowl::CustomError if:
     *   - `range_str` is not a valid range string
     *   - one of the cpuids in the range string is not a valid cpuid for this CPU.
     */
    static bowl::Expected<CpuSet, bowl::CustomError> from_range_str(std::string range_str)
    {
        CpuSet res;

        CHECK_ASSIGN(cpunr, set_from_range_str(range_str));

        for (const auto& elem : cpunr)
        {
            CHECK_ASSIGN(cpu, Cpu::from_int(elem));
            res.cpus_.emplace(cpu);
        }
        return res;
    }

    /*
     * Returns a string representation of the given CpuSet.
     *
     * This does not care for optimization currently. Every CpuSet is printed as
     * 1,2,3,...,n-1,n even if 1-n would be possible.
     */
    std::string to_str() const
    {
        std::set<int> cpunrs_;

        for (const auto& cpu : cpus_)
        {
            cpunrs_.emplace(cpu.as_int());
        }
        return fmt::format("{}", fmt::join(cpunrs_, ","));
    }

    /*
     * Creates the union of the two CpuSets:
     *
     * Example:
     *   - {1, 3}.union_with({3,5}) = {1,3,5}
     */
    CpuSet union_with(CpuSet other)
    {
        CpuSet res;
        res.cpus_ = this->cpus_;
        res.cpus_.insert(other.cpus_.begin(), other.cpus_.end());

        return res;
    }

    /*
     * Adds the given Cpu to the CpuSet:
     * Example:
     *   - {1,2,3}.add(4) = {1,2,3,4}
     */
    void add(Cpu other)
    {
        cpus_.emplace(other);
    }

    /*
     * Returns true if this CpuSet contains the given Cpu
     */
    bool contains(Cpu cpu) const
    {
        return cpus_.count(cpu) == 1;
    }

    size_t size() const
    {
        return cpus_.size();
    }

    /*
     * Iterator to the underlying std::set
     */
    std::set<Cpu>::iterator begin()
    {
        return cpus_.begin();
    }

    std::set<Cpu>::iterator end()
    {
        return cpus_.end();
    }

private:
    std::set<Cpu> cpus_;
};
} // namespace penguinxx
