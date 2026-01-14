// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de>

#pragma once

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/maybe_error.hpp>

#include <filesystem>
#include <fstream>
#include <set>
#include <vector>

#include <cstdint>

#include <fmt/format.h>

namespace penguinxx
{

/**
 *
 * For an enum type T and a given list of values of this enum
 * {val1, val2, ...}
 *
 * returns the value cast<int>(val1) | cast<int>(val2) ...
 * as it is often required for C flag types.
 */
template <class T>
int or_enum_vals(std::vector<T> enum_vals)
{
    int i = 0;
    for (const auto& val : enum_vals)
    {
        i |= static_cast<int>(val);
    }

    return i;
}

/*
 * Reads a single item from `path` into a return value of type T using operator>>.
 *
 * This is most useful for reading files in the /sys or /proc that contain only a single
 * value of a basic type, such as:
 *
 * /sys/devices/system/cpu/enabled (e.g. "0-15") -- as string
 * /sys/devices/system/cpu/cpufreq/boost (e.q. "1") -- int
 *
 * Returns a bowl::CustomError if:
 *   - `path` is not readable
 *   - the content of `path` can not be read into a return value of type T
 */
template <class T>
inline bowl::Expected<T, bowl::CustomError> read_from_file(std::filesystem::path path)
{
    std::fstream fs(path, std::fstream::in);

    if (!fs.good())
    {
        return bowl::Unexpected(bowl::CustomError(fmt::format("Could not open {}", path.c_str())));
    }

    T value;
    fs >> value;

    if (!fs)
    {
        return bowl::Unexpected(bowl::CustomError(fmt::format("Could not read {}", path.c_str())));
    }

    return value;
}

/*
 *
 * Write a single value `data` of type T to the file pointed to by `path`.
 *
 * This is basically the reverse to read_from_file, a use case might be writing:
 *
 * "(int)0" -> /sys/devices/system/cpu/cpufreq/turbo
 *
 * to disable turbo frequencies.
 *
 * Fails if:
 *   - `path` can not be opened for writing
 *   - `data` can not be written to `path`
 */
template <class T>
inline bowl::MaybeError<bowl::CustomError> write_to_file(std::filesystem::path path, T data)
{
    std::fstream fs(path, std::fstream::out);
    if (!fs)
    {
        return bowl::CustomError(
            fmt::format("Could not open {}: {}", path.c_str(), strerror(errno)));
    }

    fs << data;

    if (!fs)
    {
        return bowl::CustomError(fmt::format("Could not write to {}!", path.c_str()));
    }

    return {};
}

/*
 * Read a list of space separated values of type T.
 *
 * This is used in cases such as:
 *
 * /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors
 *   -> "performance powersave"
 *   -> std::vector<std::string>{"performance", "powersave"}
 *
 * /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies
 *   -> "3001000 3000000 2900000..."
 *   -> std::vector<uint64_t>{3001000, 3000000, 2900000, ...
 *
 * This fails witha bowl::CustomError if:
 *   - `path` can not be opened for reading
 *   - a vector of T's can not be read from `path`
 */
template <class T>
inline bowl::Expected<std::vector<T>, bowl::CustomError>
read_from_file_list(std::filesystem::path path)
{
    std::vector<T> res;

    std::fstream fs(path, std::fstream::in);

    if (!fs.good())
    {
        return bowl::Unexpected(bowl::CustomError(fmt::format("Could not open {}", path.c_str())));
    }

    T value;
    while (fs >> value)
    {
        res.emplace_back(value);
    }

    if (fs.fail() && !fs.eof())
    {
        return bowl::Unexpected(bowl::CustomError(fmt::format("Could not read {}", path.c_str())));
    }

    return res;
}

/*
 * Parses a "range string" `list` into a set of int64_t that is represented it.
 *
 * A range string is a string that is composed out of comma separated terms that are
 * either single numbers ("42") or ranges of numbers ("10-20").
 *
 * E.g.:
 *   - "1" -> {1}
 *   - "1,2,3"-> {1,2,3}
 *   - "1,4,10-13,30" -> {1, 4, 10, 11, 12, 13, 30}
 *
 * This fails with a bowl:CustomError if:
 *   - The string fails to conform to the pattern shown above. This includes things
 *     such as trailing commas or dashes, things that are not numbers, etc.
 *   - The start of X of a range X-Y is not clearly smaller than Y (X < Y)
 */
bowl::Expected<std::set<int64_t>, bowl::CustomError> set_from_range_str(const std::string list);

} // namespace penguinxx
