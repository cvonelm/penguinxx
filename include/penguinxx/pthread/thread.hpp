// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>

extern "C"
{
#include <pthread.h>
}

namespace penguinxx
{
/*
 * type of the function given to Pthread::create()
 */
typedef void* (*thread_function)(void*);

class Pthread
{
public:
    /*
     * Creates and immediately starts a new thread using pthread_create.
     *
     * The new thread calls func(arg);
     *
     * On success, returns the Pthread handle to the new thread
     *
     * On error, returns ErrnoError
     */
    static bowl::Expected<Pthread, bowl::ErrnoError> create(thread_function func, void* arg = NULL)
    {
        Pthread res;

        if (pthread_create(&res.thread_, NULL, func, arg) != 0)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return res;
    }

    /*
     * Returns the Pthread handle to the current thread.
     */
    static bowl::Expected<Pthread, bowl::ErrnoError> self()
    {
        Pthread res;

        res.thread_ = pthread_self();
        return res;
    }

    /*
     * Waits for the given Pthread to finish executing.
     *
     * Returns ErrnoError if pthread_join fails.
     */
    bowl::MaybeError<bowl::ErrnoError> join()
    {
        // TODO support return typ FreshRSS e
        if (pthread_join(thread_, NULL) != 0)
        {
            return bowl::ErrnoError();
        }

        return {};
    }

private:
    pthread_t thread_;
};
} // namespace penguinxx
