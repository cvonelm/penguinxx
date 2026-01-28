// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>
#include <functional>
#include <memory>
#include <utility>

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
    static bowl::Expected<Pthread, bowl::ErrnoError> create_raw(thread_function func,
                                                                void* arg = NULL)
    {
        Pthread res;

        if (pthread_create(&res.thread_, NULL, func, arg) != 0)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return res;
    }

    template <class F, class... Args>
    static bowl::Expected<Pthread, bowl::ErrnoError> create(F&& function, Args&&... args)
    {
        Pthread res;

        typedef std::tuple<F, Args...> invoke_tuple;

        // Pack the function and parameters into a magic tuple to
        // squeeze them through pthread's void* parameter.
        std::unique_ptr<invoke_tuple> arg = std::unique_ptr<invoke_tuple>(
            new invoke_tuple(invoke_tuple(std::move(function), std::forward<Args>(args)...)));

        if (pthread_create(&res.thread_, NULL, executor<invoke_tuple>, arg.get()) != 0)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        arg.release();

        return res;
    }

    template <class invoke_tuple>
    static void* executor(void* arg)
    {
        std::unique_ptr<invoke_tuple> func_arg(static_cast<invoke_tuple*>(arg));

        // Tuple is unpacked here. Don't ask how.
        using indices = std::make_index_sequence<std::tuple_size<invoke_tuple>::value - 1>;
        executor_helper(*func_arg.get(), indices{});

        return nullptr;
    }

    template <class F, class... Args, size_t... Indices>
    static void executor_helper(std::tuple<F, Args...>& t, std::index_sequence<Indices...> is)
    {
        std::invoke(std::move(std::get<0>(t)), std::move(std::get<Indices + 1>(t))...);
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
