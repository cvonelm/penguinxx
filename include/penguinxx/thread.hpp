#pragma once

#include <penguinxx/process.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>

#include <string>

extern "C"
{
#include <sys/types.h>
}

namespace penguinxx
{
// A class for Threads
class Thread
{
public:
    // TODO Currently we only allow complete process-thread pairs
    explicit Thread(Process parent, pid_t tid) : parent_(parent), tid_(tid)
    {
    }

    // Get the name of the Thread
    //
    // Returns bowl::CustomError if reading /proc/{parent_}/task/{tid_}/comm
    // fails
    bowl::Expected<std::string, bowl::CustomError> comm() const
    {
        return read_from_file<std::string>(proc_path() / "comm");
    }

    int as_pid_t() const
    {
        return tid_;
    }

    friend bool operator<(const Thread& lhs, const Thread& rhs)
    {

        return lhs.tid_ < rhs.tid_;
    }

    friend bool operator==(const Thread& lhs, const Thread& rhs)
    {

        return lhs.tid_ == rhs.tid_;
    }

    friend bool operator!=(const Thread& lhs, const Thread& rhs)
    {

        return lhs.tid_ != rhs.tid_;
    }

private:
    std::filesystem::path proc_path() const
    {
        return std::filesystem::path("/proc") / std::to_string(parent_.as_pid_t()) / "task" /
               std::to_string(tid_);
    }

    Process parent_;
    pid_t tid_;
};

inline Thread Process::as_thread() const
{
    return Thread(*this, as_pid_t());
}
} // namespace penguinxx
