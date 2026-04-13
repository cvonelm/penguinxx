// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/fd/error.hpp>
#include <penguinxx/fd/fd.hpp>
#include <penguinxx/util.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>

#include <vector>

#include <cstdint>

extern "C"
{
#include <sys/eventfd.h>
}

namespace penguinxx
{
class EventFd : public Fd
{
public:
    enum class Flags : int
    {
        CLOEXEC = EFD_CLOEXEC,
        NONBLOCK = EFD_NONBLOCK,
        SEMAPHORE = EFD_SEMAPHORE
    };

    static bowl::Expected<EventFd, bowl::ErrnoError> create(int initial_value = 0,
                                                            std::vector<Flags> flags = {})
    {
        EventFd fd;
        fd.fd_ = eventfd(initial_value, or_enum_vals(flags));

        if (fd.fd_ == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return fd;
    }

    bowl::MaybeError<bowl::ErrnoError> write(uint64_t value)
    {
        if (::write(fd_, &value, sizeof(value)) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }

    bowl::Expected<uint64_t, bowl::ErrnoError> read()
    {
        uint64_t res = 0;

        if (::read(fd_, &res, sizeof(res)) != sizeof(res))
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return res;
    }
};
} // namespace penguinxx
