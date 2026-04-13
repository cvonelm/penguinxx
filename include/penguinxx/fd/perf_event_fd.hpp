// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/cpu.hpp>
#include <penguinxx/fd/fd.hpp>
#include <penguinxx/thread.hpp>

#include <algorithm>
#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <cstdint>
#include <variant>
#include <vector>

extern "C"
{
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
}

namespace penguinxx
{

// TODO: Document
class PerfEventFd : public Fd
{
public:
    template <class T>
    bowl::Expected<T, bowl::ErrnoError> read()
    {
        static_assert(std::is_pod_v<T> == true);
        T val;

        if (::read(fd_, &val, sizeof(val)) == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return val;
    }

    bowl::Expected<uint64_t, bowl::ErrnoError> get_stream_id()
    {
        uint64_t id = 0;
        if (ioctl(fd_, PERF_EVENT_IOC_ID, &id) == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return id;
    }

    bowl::MaybeError<bowl::ErrnoError> enable()
    {
        if (ioctl(fd_, PERF_EVENT_IOC_ENABLE) == -1)
        {
            return bowl::ErrnoError();
        }

        return {};
    }

    bowl::MaybeError<bowl::ErrnoError> disable()
    {
        if (ioctl(fd_, PERF_EVENT_IOC_DISABLE) == -1)
        {
            return bowl::ErrnoError();
        }

        return {};
    }

    static bowl::Expected<PerfEventFd, bowl::ErrnoError>
    create(struct perf_event_attr* perf_attr, std::variant<Thread, Cpu> loc, WeakFd group_fd,
           unsigned long flags, WeakFd cgroup_fd)
    {
        PerfEventFd res;

        int cpuid = -1;
        pid_t pid = -1;

        if (std::holds_alternative<Cpu>(loc))
        {
            if (!cgroup_fd.invalid())
            {
                pid = cgroup_fd.as_int();
                flags |= PERF_FLAG_PID_CGROUP;
            }

            cpuid = std::get<Cpu>(loc).as_int();
        }
        else
        {
            pid = std::get<Thread>(loc).as_pid_t();
        }

        if ((res.fd_ = syscall(__NR_perf_event_open, perf_attr, pid, cpuid, group_fd.as_int(),
                               flags)) == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return res;
    }

    bowl::MaybeError<bowl::ErrnoError> set_output_to(WeakFd other)
    {
        if (ioctl(fd_, PERF_EVENT_IOC_SET_OUTPUT, other.as_int()) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }

    bowl::MaybeError<bowl::ErrnoError>
    set_syscall_filter(const std::vector<int64_t>& syscall_filter)
    {
        if (syscall_filter.empty())
        {
            return {};
        }

        std::vector<std::string> names;
        std::transform(syscall_filter.cbegin(), syscall_filter.end(), std::back_inserter(names),
                       [](const auto& elem) { return "id == " + elem; });

        std::string filter = names.front();

        for (auto it = (names.begin() + 1); it != names.end(); it++)
        {
            filter = filter + "||" + *it;
        }

        if (ioctl(fd_, PERF_EVENT_IOC_SET_FILTER, filter.c_str()) == -1)
        {
            return bowl::ErrnoError();
        }

        return {};
    }
};
} // namespace penguinxx
