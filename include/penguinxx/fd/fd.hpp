// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>

extern "C"
{
#include <fcntl.h>
#include <unistd.h>
}

namespace penguinxx
{
class Fd;

class WeakFd
{
public:
    explicit WeakFd(int fd) : fd_(fd)
    {
    }

    [[nodiscard]] int as_int() const
    {
        return fd_;
    }

    static WeakFd make_invalid()
    {
        return WeakFd(-1);
    }

    bool invalid()
    {
        return fd_ == -1;
    }

    friend bool operator==(const WeakFd& lhs, const Fd& rhs);
    friend bool operator==(const Fd& lhs, const WeakFd& rhs);

    friend bool operator!=(const WeakFd& lhs, const Fd& rhs);
    friend bool operator!=(const Fd& lhs, const WeakFd& rhs);

    friend bool operator<(const WeakFd& lhs, const Fd& rhs);
    friend bool operator<(const Fd& lhs, const WeakFd& rhs);

    friend bool operator==(const WeakFd& lhs, const WeakFd& rhs)
    {
        return lhs.fd_ == rhs.fd_;
    }

    friend bool operator<(const WeakFd& lhs, const WeakFd& rhs)
    {
        return lhs.fd_ < rhs.fd_;
    }

    friend inline std::ostream& operator<<(std::ostream& os, const WeakFd& fd)
    {
        return os << std::string("fd") << std::to_string(fd.fd_);
    }

private:
    int fd_ = -1;
};

class Fd
{
public:
    Fd(Fd&) = delete;
    Fd& operator=(Fd&) = delete;

    Fd(Fd&& other) noexcept : fd_(other.fd_)

    {
        other.fd_ = -1;
    }

    Fd& operator=(Fd&& other) noexcept
    {
        this->fd_ = other.fd_;
        other.fd_ = -1;
        return *this;
    }

    [[nodiscard]] WeakFd to_weak() const
    {
        return WeakFd(fd_);
    }

    ~Fd()
    {
        if (fd_ != -1)
        {
            close(fd_);
        }
        fd_ = -1;
    }

    friend bool operator==(const Fd& lhs, const Fd& rhs)
    {
        return lhs.fd_ == rhs.fd_;
    }

    friend bool operator==(const WeakFd& lhs, const Fd& rhs);
    friend bool operator!=(const WeakFd& lhs, const Fd& rhs);

    static Fd from_int(int fd)
    {
        Fd res;
        res.fd_ = fd;
        return res;
    }

protected:
    bool invalid()
    {
        return fd_ == -1;
    }

    Fd() = default;

    int fd_ = -1; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
};
} // namespace penguinxx
