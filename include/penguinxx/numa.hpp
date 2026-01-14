// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/cpu.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>

extern "C"
{
#include <numa.h>
}

namespace penguinxx
{

/*
 * Type for a specific NUMA node assignment
 */
class NUMANodeSet
{
public:
    NUMANodeSet(NUMANodeSet&) = delete;
    NUMANodeSet& operator=(NUMANodeSet&) = delete;

    NUMANodeSet(NUMANodeSet&& other)
    {
        this->mask_ = other.mask_;
        other.mask_ = nullptr;
    }

    NUMANodeSet& operator=(NUMANodeSet&& other)
    {
        this->mask_ = other.mask_;
        other.mask_ = nullptr;
        return *this;
    }

    /*
     * Get the NUMANodeSet for a specific Cpu.
     *
     * Returns bowl::ErrnoError if:
     *   - numa_node_of_cpu can't get the NUMA Node of the current CPU
     *   - NUMANodeSet::alloc can not alloc the underlying numa_bitmask
     */
    static bowl::Expected<NUMANodeSet, bowl::ErrnoError> of_cpu(Cpu cpu)
    {
        int numa_node = numa_node_of_cpu(cpu.as_int());

        if (numa_node == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        CHECK_ASSIGN(mask, NUMANodeSet::alloc());

        numa_bitmask_clearall(mask.mask_);
        numa_bitmask_setbit(mask.mask_, numa_node);
        return mask;
    }

    /*
     * Returns a NUMANodeSet representing any NUMA node in the system
     *
     * Fails with bowl::ErrnoError if:
     *   - NUMANodeSet::aloc() fails to allocate the underlying
     *     numa_bitmask.
     */
    static bowl::Expected<NUMANodeSet, bowl::ErrnoError> all()
    {
        CHECK_ASSIGN(mask, NUMANodeSet::alloc());

        numa_bitmask_clearall(mask.mask_);
        numa_bitmask_setall(mask.mask_);

        return mask;
    }

    /*
     * Binds the current thread to only allocate memory from
     * given NUMANodeSet.
     */
    void membind()
    {
        numa_set_membind(mask_);
    }

    ~NUMANodeSet()
    {
        if (mask_ != nullptr)
        {
            numa_bitmask_free(mask_);
        }
    }

private:
    NUMANodeSet()
    {
    }

    /*
     * Allocates the underlying numa_bitmask.
     *
     * This fails with bowl::ErrnoError if:
     *   - numa_bitmask_alloc can not alloc the memory.
     */
    static bowl::Expected<NUMANodeSet, bowl::ErrnoError> alloc()
    {
        NUMANodeSet mask;

        mask.mask_ = numa_bitmask_alloc(numa_max_possible_node());

        if (mask.mask_ == nullptr)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return mask;
    }

    bitmask* mask_ = nullptr;
};
} // namespace penguinxx
