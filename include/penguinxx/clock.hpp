// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>

#include <cstdint>

extern "C"
{
#include <time.h>
};

namespace penguinxx
{
enum class Clocks : int
{
    REALTIME = CLOCK_REALTIME,
    MONOTONIC = CLOCK_MONOTONIC,
    PROCESS_CPUTIME_ID = CLOCK_PROCESS_CPUTIME_ID,
    THREAD_CPUTIME_ID = CLOCK_THREAD_CPUTIME_ID,
    MONOTONIC_RAW = CLOCK_MONOTONIC_RAW,
    REALTIME_COARSE = CLOCK_REALTIME_COARSE,
    MONOTONIC_COARSE = CLOCK_MONOTONIC_COARSE,
    BOOTTIME = CLOCK_BOOTTIME,
    REALTIME_ALARM = CLOCK_REALTIME_ALARM,
    BOOTTIME_ALARM = CLOCK_BOOTTIME_ALARM,
    TAI = CLOCK_TAI
};

class Clock
{
public:
    /*
     * Returns the value of the given clock in nanoseconds.
     *
     * Return bowl::ErrnoError if clock_gettime() fails.
     */
    static bowl::Expected<uint64_t, bowl::ErrnoError> gettime(Clocks clock)
    {
        struct timespec sp;

        if (clock_gettime(static_cast<int>(clock), &sp) == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return sp.tv_sec * NSEC_IN_SEC + sp.tv_nsec;
    }

private:
    static const uint64_t NSEC_IN_SEC = 1000000000;
};
} // namespace penguinxx
