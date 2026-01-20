// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/util.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>

#include <filesystem>

extern "C"
{
#include <sys/types.h>
#include <unistd.h>
}

namespace penguinxx
{
class Process
{
public:
    static Process me()
    {
        return Process(getpid());
    }

    bowl::Expected<std::string, bowl::CustomError> comm()
    {
        return read_from_file<std::string>(proc_path() / "comm");
    }

    pid_t as_pid_t()
    {
        return pid_;
    }

private:
    Process(pid_t pid) : pid_(pid)
    {
    }

    std::filesystem::path proc_path()
    {
        return std::filesystem::path("/proc") / std::to_string(pid_);
    }

    pid_t pid_;
};
}; // namespace penguinxx
