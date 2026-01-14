// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>
#include <bowl/unexpected.hpp>

extern "C"
{
#include <pthread.h>
}

namespace penguinxx
{

/*
 * Wrapper type for a pthread barrier.
 *
 * A barrier is an object that only lets threads pass after a
 * pre-defined number of threads have entered.
 *
 * This makes it useful for synchronization purposes.
 */
class Barrier
{
public:
    /*
     * Creates a barrier that lets threads through if `num_threads` threads
     * have entered it.
     *
     * Returns bowl::ErrnoError if pthread_barrier_init fails.
     */
    static bowl::Expected<Barrier, bowl::ErrnoError> create(int num_threads)
    {
        Barrier res;

        if (pthread_barrier_init(&res.barrier_, NULL, num_threads) != 0)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return res;
    }

    /*
     * Enters this Barrier, waiting in it until the pre-defined number of
     * threads have entered it.
     *
     * Returns bowl::ErrnoError if pthread_barrier_wait fails.
     */
    bowl::MaybeError<bowl::ErrnoError> wait()
    {
        auto res = pthread_barrier_wait(&barrier_);

        if (res != PTHREAD_BARRIER_SERIAL_THREAD && res != 0)
        {
            return bowl::ErrnoError();
        }

        return {};
    }

    ~Barrier()
    {
        pthread_barrier_destroy(&barrier_);
    }

private:
    Barrier()
    {
    }

    pthread_barrier_t barrier_;
};
} // namespace penguinxx
