// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de
#pragma once

#include <penguinxx/fd/fd.hpp>
#include <penguinxx/memory_map.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>

#include <cstdint>

extern "C"
{
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
}

namespace penguinxx
{

class MemFd : public Fd
{
public:
    enum class Seal
    {
        ALLOW_SEAL,
        DENY_SEAL
    };

    static bowl::Expected<MemFd, bowl::ErrnoError> create(std::string name,
                                                          Seal allow_sealing = Seal::DENY_SEAL)
    {
        MemFd res;

        int arg = 0;
        if (allow_sealing == Seal::ALLOW_SEAL)
        {
            arg = MFD_ALLOW_SEALING;
        }

        res.fd_ = memfd_create(name.c_str(), arg);
        if (res.fd_ == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }
        return res;
    }

    bowl::MaybeError<bowl::ErrnoError> seal_grow()
    {
        if (fcntl(fd_, F_ADD_SEALS, F_SEAL_GROW) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }

    bowl::MaybeError<bowl::ErrnoError> seal_shrink()
    {
        if (fcntl(fd_, F_ADD_SEALS, F_SEAL_SHRINK) == -1)
        {
            return bowl::ErrnoError();
        }

        return {};
    }

    [[nodiscard]] bowl::MaybeError<bowl::ErrnoError> seal_sealing()

    {
        if (fcntl(fd_, F_ADD_SEALS, F_SEAL_SEAL) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }

    bowl::MaybeError<bowl::ErrnoError> seal_write()
    {
        if (fcntl(fd_, F_ADD_SEALS, F_SEAL_WRITE) == -1)
        {
            return bowl::ErrnoError();
        }

        seal_write_ = true;
        return {};
    }

    bowl::MaybeError<bowl::ErrnoError> set_size(uint64_t size)
    {
        if (ftruncate(fd_, size) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }

    bowl::Expected<MemoryMap, bowl::ErrnoError> map(size_t size, off_t offset = 0,
                                                    void* location = nullptr)
    {
        int prot = 0;
        if (seal_write_)
        {
            prot = PROT_READ;
        }
        else
        {
            prot = PROT_READ | PROT_WRITE;
        }

        return MemoryMap::create(to_weak(), size, prot, offset, location);
    }

private:
    bool seal_write_ = false;
};

} // namespace penguinxx
