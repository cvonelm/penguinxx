// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/syscalls.hpp>
#include <penguinxx/util.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/macros.hpp>

#include <filesystem>

extern "C"
{
#include <sys/types.h>
#include <unistd.h>
}

namespace penguinxx
{

class Thread;

class Process
{
public:
    // Get the current Process
    static Process me()
    {
        return { getpid() };
    }

    // Get the name of the given Process
    //
    // Returns bowl::CustomError if reading /proc/{pid}/comm failed
    [[nodiscard]] bowl::Expected<std::string, bowl::CustomError> comm() const
    {
        return read_from_file<std::string>(proc_path() / "comm");
    }

    // Get the path to the executable of the Process.
    //
    // Returns bowl::CustomError if reading that path failed
    [[nodiscard]] bowl::Expected<std::filesystem::path, bowl::CustomError> exe() const
    {
        auto res = Syscalls::readlink(proc_path() / "exe");

        if (!res.ok())
        {
            return bowl::Unexpected(bowl::CustomError(res.unpack_error().display()));
        }

        return res.unpack_ok();
    }

    [[nodiscard]] bowl::Expected<std::vector<std::string>, bowl::CustomError> cmdline() const
    {
        std::vector<std::string> res;
        // cmdline_str contains the \0 separated arguments to the process.
        CHECK_ASSIGN(cmdline_str, read_from_file<std::string>(proc_path() / "cmdline"));

        auto* cmdline_cur_cstr = cmdline_str.c_str();
        while (cmdline_cur_cstr <= cmdline_str.c_str() + cmdline_str.size())
        {
            res.emplace_back(cmdline_cur_cstr);
            cmdline_cur_cstr = cmdline_cur_cstr + strlen(cmdline_cur_cstr) + 1;
        }

        return res;
    }

    [[nodiscard]] pid_t as_pid_t() const
    {
        return pid_;
    }

    Process(pid_t pid) : pid_(pid)
    {
    }

    [[nodiscard]] Thread as_thread() const;

    friend bool operator<(const Process& lhs, const Process& rhs)
    {

        return lhs.pid_ < rhs.pid_;
    }

    friend bool operator==(const Process& lhs, const Process& rhs)
    {

        return lhs.pid_ == rhs.pid_;
    }

    friend bool operator!=(const Process& lhs, const Process& rhs)
    {

        return lhs.pid_ != rhs.pid_;
    }

private:
    [[nodiscard]] std::filesystem::path proc_path() const
    {
        return std::filesystem::path("/proc") / std::to_string(pid_);
    }

    pid_t pid_;
};
}; // namespace penguinxx
