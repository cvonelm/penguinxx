// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#include <penguinxx/cpu.hpp>
#include <penguinxx/topology.hpp>

#include <bowl/error.hpp>
#include <bowl/maybe_error.hpp>

namespace penguinxx
{

bowl::Expected<Governor, bowl::CustomError> string_to_governor(std::string str)
{
    if (str == "performance")
    {
        return Governor::PERFORMANCE;
    }
    else if (str == "powersave")
    {
        return Governor::POWERSAVE;
    }
    else if (str == "userspace")
    {
        return Governor::USERSPACE;
    }
    else if (str == "ondemand")
    {
        return Governor::ONDEMAND;
    }
    else if (str == "conservative")
    {
        return Governor::CONSERVATIVE;
    }
    else if (str == "schedutil")
    {
        return Governor::SCHEDUTIL;
    }
    else
    {
        return bowl::Unexpected(bowl::CustomError(fmt::format("Unknown governor: {}", str)));
    }
}

std::string governor_to_string(Governor gov)
{
    switch (gov)
    {
    case penguinxx::Governor::POWERSAVE:
        return "powersave";
    case Governor::CONSERVATIVE:
        return "conservative";
    case Governor::USERSPACE:
        return "userspace";
    case Governor::PERFORMANCE:
        return "performance";
    case Governor::SCHEDUTIL:
        return "schedutil";
    case penguinxx::Governor::ONDEMAND:
        return "ondemand";
    }

    throw std::runtime_error(
        fmt::format("Unknown governor {}, this is an error!", static_cast<int>(gov)));
}

bowl::Expected<TurboState, bowl::CustomError> int_to_turbo_state(uint64_t state)
{
    switch (state)
    {
    case 0:
        return TurboState::OFF;
    case 1:
        return TurboState::ON;
    }
    return bowl::Unexpected(
        bowl::CustomError(fmt::format("Unexpected turbo boost state: {}", state)));
}

uint64_t turbo_state_to_int(TurboState state)
{
    switch (state)
    {
    case TurboState::ON:
        return 1;
    case TurboState::OFF:
        return 0;
    }

    throw std::runtime_error(fmt::format("Unknown TurboState enum value \"{}\", this is an error!",
                                         static_cast<int>(state)));
}

bowl::Expected<Cpu, bowl::CustomError> Cpu::from_int(int cpuid)
{
    Cpu res(cpuid);
    if (!CpuTopology::instance().cpus().contains(res))
    {
        return bowl::Unexpected(
            bowl::CustomError(fmt::format("No CPU {} on this processor!", cpuid)));
    }

    return Cpu(cpuid);
}

uint64_t cstate_state_to_cpuidle_disable(CStateState state)
{
    switch (state)
    {
    case CStateState::ENABLED:
        return 0;
    case CStateState::DISABLED:
        return 1;
    }
    throw std::runtime_error(fmt::format("Unknown CStateState enum value \"{}\", this is an error!",
                                         static_cast<int>(state)));
}

bowl::Expected<CStateState, bowl::CustomError> cpuidle_disable_to_cstate_state(uint64_t state)
{
    switch (state)
    {
    case 0:
        return CStateState::ENABLED;
    case 1:
        return CStateState::DISABLED;
    }

    return bowl::Unexpected(
        bowl::CustomError(fmt::format("Unknown cstate disable setting: {}", state)));
}
} // namespace penguinxx
