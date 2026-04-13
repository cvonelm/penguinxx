// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>

extern "C"
{
#include <unistd.h>
}

namespace penguinxx
{
class Sysconf
{
public:
    static bowl::Expected<long, bowl::ErrnoError> pagesize()
    {
        long pagesize = sysconf(_SC_PAGESIZE);

        if (pagesize == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return pagesize;
    }
};
} // namespace penguinxx
