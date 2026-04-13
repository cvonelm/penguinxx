// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/util.hpp>

#include <bowl/error.hpp>
#include <bowl/unexpected.hpp>

#include <string>

#include <fmt/format.h>

namespace penguinxx
{

bowl::Expected<std::set<int64_t>, bowl::CustomError> set_from_range_str(const std::string list)
{
    constexpr int BASE10 = 10;
    std::set<int64_t> res;

    std::string part;

    const char* pos = list.c_str();
    const char* end = pos + list.length();
    while (pos < end)
    {
        char* newpos = nullptr;
        int64_t num = strtoll(pos, &newpos, BASE10);
        if (pos == newpos)
        {
            return bowl::Unexpected(bowl::CustomError(
                fmt::format("can not parse beginning of \"{}\" as number!", std::string(pos))));
        }

        if (*newpos == ',')
        {
            res.emplace(num);
            pos = newpos + 1;
            continue;
        }
        else if (*newpos == 0)
        {
            res.emplace(num);
            break;
        }
        else if (*newpos == '-')
        {
            newpos = newpos + 1;

            if (newpos >= end)
            {
                return bowl::Unexpected(bowl::CustomError(fmt::format(
                    "'{}' ends right after '-' character, expected another number!", list)));
            }

            pos = newpos;

            int64_t end = strtoll(pos, &newpos, BASE10);

            if (pos == newpos)
            {
                return bowl::Unexpected(bowl::CustomError(
                    fmt::format("Can not parse {} as a number!", std::string(pos))));
            }

            if (end <= num)
            {
                return bowl::Unexpected(bowl::CustomError(fmt::format(
                    "Invalid range {}-{} in {}, end must be larger than start!", num, end, list)));
            }

            for (; num <= end; num++)
            {
                res.emplace(num);
            }

            if (*newpos == ',')
            {
                pos = newpos + 1;
                continue;
            }
            else if (*newpos == 0)
            {
                break;
            }
            else
            {
                return bowl::Unexpected(bowl::CustomError(fmt::format(
                    "Unexpected character {} at postion {} in {}, expected '\\0' or ','!", *newpos,
                    newpos - list.c_str(), list)));
            }
        }
        else
        {
            return bowl::Unexpected(bowl::CustomError(
                fmt::format("Unexpected character {} at position {} in {}, expected ',' or '-'!",
                            *newpos, newpos - list.c_str(), list)));
        }
    }

    return res;
}

} // namespace penguinxx
