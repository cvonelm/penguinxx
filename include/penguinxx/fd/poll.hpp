// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/maybe_error.hpp>

#include <penguinxx/fd/fd.hpp>
#include <vector>

extern "C"
{
#include <poll.h>
}

namespace penguinxx
{
class Poll
{
public:
    Poll() = default;

    /*
     * Adds the weak File Descriptor reference `fd`
     * with the trigger events `events` to the
     * polled File Descriptors
     *
     * TODO: replace events with std::vector<enum> something
     */
    void add_fd(WeakFd fd, short events)
    {
        struct pollfd new_elem;
        new_elem.fd = fd.as_int();
        new_elem.events = events;
        new_elem.revents = events;
        pfds_.emplace_back(new_elem);
    }

    bowl::MaybeError<bowl::ErrnoError> poll()
    {
        if (::poll(pfds_.data(), pfds_.size(), -1) == -1)
        {
            return bowl::ErrnoError();
        }
        return {};
    }

    const std::vector<struct pollfd>& fds()
    {
        return pfds_;
    }

private:
    std::vector<struct pollfd> pfds_;
};
} // namespace penguinxx
