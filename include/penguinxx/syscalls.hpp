// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/macros.hpp>

#include <filesystem>
#include <vector>

extern "C"
{
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>
}

namespace penguinxx
{
// Type-safe and non-throw wrappers around the underlying system calls.
class Syscalls
{
public:
    // For `path`, if it is a symbolic link, return the destination of the link.
    //
    // Return bowl::ErrnoError if:
    // - `path` is not pointing to as symbolic link
    // - `lstat` failed for some reason on `path`
    // - `readlink` failed for some reason on `path`
    static bowl::Expected<std::filesystem::path, bowl::ErrnoError>
    readlink(std::filesystem::path path)
    {
        CHECK_ASSIGN(stat, lstat(path));

        std::vector<char> buf;

        if (stat.st_size == 0)
        {
            buf.reserve(PATH_MAX);
        }
        else
        {
            buf.reserve(stat.st_size + 1);
        }

        if (::readlink(path.c_str(), buf.data(), buf.capacity()) == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return std::filesystem::path(buf.data());
    }

    // TODO Wrapper around stat structure
    //
    // For `path` return a struct stat containing useful information about the symbolic
    // link given in `path`.
    //
    // Returns bowl::ErrnoError if lstat fails.
    static bowl::Expected<struct stat, bowl::ErrnoError> lstat(std::filesystem::path path)
    {
        struct stat ret; // NOLINT(cppcoreguidelines-pro-type-member-init)

        if (::lstat(path.c_str(), &ret) == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return ret;
    }
};
} // namespace penguinxx
