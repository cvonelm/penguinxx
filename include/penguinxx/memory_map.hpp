// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include "bowl/error.hpp"
#include "bowl/expected.hpp"
#include "bowl/unexpected.hpp"
#include "penguinxx/fd/fd.hpp"

extern "C"
{
#include <sys/mman.h>
#include <sys/types.h>
}

namespace penguinxx
{
class MemoryMap
{
public:
    static bowl::Expected<MemoryMap, bowl::ErrnoError> create(WeakFd fd, size_t size,
                                                              int prot = PROT_READ | PROT_WRITE,
                                                              off_t offset = 0,
                                                              void* location = nullptr)
    {

        void* addr = nullptr;
        if (location == nullptr)
        {
            addr = mmap(nullptr, size, prot, MAP_SHARED, fd.as_int(), offset);
        }
        else
        {
            addr = mmap(location, size, prot, MAP_SHARED | MAP_FIXED, fd.as_int(), offset);
        }

        if (addr == MAP_FAILED)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return MemoryMap(addr, size);
    }

    MemoryMap() = default;

    MemoryMap(MemoryMap&) = delete;
    MemoryMap& operator=(MemoryMap&) = delete;

    MemoryMap(MemoryMap&& other) noexcept : addr_(other.addr_), size_(other.size_)
    {
        other.addr_ = nullptr;
    }

    MemoryMap& operator=(MemoryMap&& other) noexcept
    {
        unmap();
        addr_ = other.addr_;
        size_ = other.size_;

        other.addr_ = nullptr;

        return *this;
    }

    template <typename T>
    [[nodiscard]] T* as()
    {
        if (sizeof(T) > size_)
        {
            return nullptr;
        }
        return reinterpret_cast<T*>(addr_);
    }

    template <typename T>
    [[nodiscard]] const T* as() const
    {
        if (sizeof(T) > size_)
        {
            return nullptr;
        }
        return reinterpret_cast<const T*>(addr_);
    }

    ~MemoryMap()
    {
        unmap();
    }

    size_t size() const
    {
        return size_;
    }

private:
    MemoryMap(void* addr, size_t size) : addr_(addr), size_(size)
    {
    }

    void unmap()
    {
        if (addr_ != nullptr)
        {
            munmap(addr_, size_);
        }
    }

    void* addr_ = nullptr;
    size_t size_ = 0;
};
} // namespace penguinxx
