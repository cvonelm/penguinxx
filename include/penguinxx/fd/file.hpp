// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/fd/error.hpp>
#include <penguinxx/fd/fd.hpp>
#include <penguinxx/util.hpp>

#include <vector>

extern "C"
{
#include <fcntl.h>
}

namespace penguinxx
{
class FileFd : public Fd
{
    enum class Flags : int
    {

        ACCMODE = O_ACCMODE,
        RDONLY = O_RDONLY,
        WRONLY = O_WRONLY,
        RDWR = O_RDWR,
        CREAT = O_CREAT,
        NOCTTY = O_NOCTTY,
        TRUNC = O_TRUNC,
        APPEND = O_APPEND,
        NONBLOCK = O_NONBLOCK,
        NDELAY = O_NDELAY,
        SYNC = O_SYNC,
        FSYNC = O_FSYNC,
        ASYNC = O_ASYNC,
        DIRECTORY = O_DIRECTORY,
        NOFOLLOW = O_NOFOLLOW,
        CLOEXEC = O_CLOEXEC,
        DIRECT = O_DIRECT,
        NOATIME = O_NOATIME,
        PATH = O_PATH,
        DSYNC = O_DSYNC,
        TMPFILE = O_TMPFILE
    };

public:
    static bowl::Expected<Flags, ConvertError<Flags>> Flags_convert(int flag)
    {
        Flags res;
        switch ((res = static_cast<Flags>(flag)))
        {
        case Flags::ACCMODE:
        case Flags::RDONLY:
        case Flags::WRONLY:
        case Flags::RDWR:
        case Flags::CREAT:
        case Flags::NOCTTY:
        case Flags::TRUNC:
        case Flags::APPEND:
        case Flags::NONBLOCK:
        // case Flags::NDELAY : Equal to NONBLOCK
        case Flags::SYNC:
        // case Flags::FSYNC : Equal to FSYNC
        case Flags::ASYNC:
        case Flags::DIRECTORY:
        case Flags::NOFOLLOW:
        case Flags::CLOEXEC:
        case Flags::DIRECT:
        case Flags::NOATIME:
        case Flags::PATH:
        case Flags::DSYNC:
        case Flags::TMPFILE:
            return res;
        }
        return bowl::Unexpected(ConvertError<Flags>(flag));
    }

    static bowl::Expected<FileFd, bowl::ErrnoError> open(std::string path,
                                                         std::vector<Flags> flags = {})
    {
        FileFd res;
        res.fd_ = ::open(path.c_str(), or_enum_vals(flags));

        if (res.fd_ == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return res;
    }

    bowl::MaybeError<bowl::ErrnoError> make_nonblock()
    {
        if (fcntl(fd_, F_SETFL, O_NONBLOCK) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }
};
} // namespace penguinxx
