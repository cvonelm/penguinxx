// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <bowl/error.hpp>

#include <fmt/format.h>

template <class T>
class ConvertError : public bowl::Error
{
public:
    ConvertError(int num) : num_(num)
    {
    }

    [[nodiscard]] std::string display() const override
    {
        return fmt::format("Can not convert {} to {}!", num_, typeid(T).name());
    }

private:
    int num_;
};
