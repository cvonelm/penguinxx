// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

#pragma once

#include <penguinxx/util.hpp>

#include <bowl/error.hpp>
#include <bowl/expected.hpp>
#include <bowl/macros.hpp>
#include <bowl/maybe_error.hpp>
#include <bowl/unexpected.hpp>

#include <fmt/format.h>

#include <filesystem>
#include <vector>

#include <cstdint>

extern "C"
{
#include <sched.h>
}

namespace penguinxx
{

/*
 * Enum of CPU governors defined as of Linux 6.18
 */
enum class Governor
{
    PERFORMANCE,
    POWERSAVE,
    USERSPACE,
    ONDEMAND,
    CONSERVATIVE,
    SCHEDUTIL
};
/*
 * Converts `str`, the name of a governor, such as ("powersave"), into
 * an enum value of the above enum.
 *
 * Returns bowl::CustomError if:
 *   - `str` is not the name of a governor
 */
bowl::Expected<Governor, bowl::CustomError> string_to_governor(std::string str);

/*
 * Converts a penguinxx::Governor into its string representation.
 */
std::string governor_to_string(Governor gov);

/*
 * The two states of CPU turbo features, ON and OFF
 */
enum class TurboState
{
    ON,
    OFF
};

/*
 * Converts a number, as read from
 *
 * /sys/devices/system/cpu/cpufreq/turbo
 *
 * into a penguinxx::TurboState
 *
 * Converts:
 *   - 1 -> TurboState::ON
 *   - 0 -> TurboState::OFF
 *
 * Returns bowl::CustomError if:
 *   - `state` is neither 0, nor 1
 */
bowl::Expected<TurboState, bowl::CustomError> int_to_turbo_state(uint64_t state);

/*
 * Converts a penguinxx::TurboState into its numeric representation
 */
uint64_t turbo_state_to_int(TurboState state);

/*
 * A CState of the processor.
 *
 * This is not an enum, because the available CStates differ by system
 */
class Cpu;

class CState
{
    friend Cpu;

public:
    friend bool operator==(const CState& lhs, const CState& rhs)
    {
        return lhs.name_ == rhs.name_;
    }

    friend bool operator<(const CState& lhs, const CState& rhs)
    {
        return lhs.name_ < rhs.name_;
    }

    std::string get_name()
    {
        return name_;
    }

private:
    explicit CState(std::string name) : name_(name)
    {
    }

    std::string name_;
};

enum class CStateState
{
    ENABLED,
    DISABLED
};

/*
 * Converts a CStateState to a value that can be written to
 * /sys/devices/system/cpu/cpu{cpuid}/cpuidle/state{x}/disable
 *
 * Important: This setting is, unlike other interface not
 * `enable`, but `disable`, so ENABLED is false and DISABLED is true.
 */
uint64_t cstate_state_to_cpuidle_disable(CStateState state);

/*
 *
 * Does the opposite conversion to the above function.
 *
 * Returns bowl::CustomError if `state` is something else than 0 or 1.
 */
bowl::Expected<CStateState, bowl::CustomError> cpuidle_disable_to_cstate_state(uint64_t state);

class CpuTopology;

/*
 * Type for expressing a single core of the current CPU, identified by its
 * cpuid.
 */
class Cpu
{
public:
    friend CpuTopology;

    /*
     * Creates a Cpu from a `cpuid`
     *
     * Returns a bowl::CustomError if:
     *   - `cpuid` is not the id of a valid Cpu in this system.
     */
    static bowl::Expected<Cpu, bowl::CustomError> from_int(int cpuid);

    /*
     * Returns the cpuid of the given Cpu
     */
    int as_int() const
    {
        return cpu_;
    }

    /*
     * Get the Cpu object for the CPU currently executing.
     *
     * Returns bowl::ErrnoError if sched_getcpu fails.
     */
    static bowl::Expected<Cpu, bowl::ErrnoError> current()
    {
        int result = sched_getcpu();

        if (result == -1)
        {
            return bowl::Unexpected(bowl::ErrnoError());
        }

        return Cpu(result);
    }

    /*
     * FREQUENCY NOTE: In penguinxx, frequency is always expressed in 1 Hz
     * and adjusted on the fly if the underlying API needs something else.
     */

    /*
     * Returns the current frequency of the given Cpu.
     *
     * Returns bowl::ErrnoError if "scaling_cur_freq" could not be read
     * for some reason.
     */
    bowl::Expected<uint64_t, bowl::CustomError> frequency()
    {

        CHECK_ASSIGN(freq, read_from_file<uint64_t>(cpufreq_path() / "scaling_cur_freq"));

        return freq * 1000;
    }

    /*
     * Sets the frequency of the given Cpu to `frequency`.
     *
     * Returns bowl::CustomError if "scaling_setspeed" could not be written.
     *
     * In general, writing to "scaling_setspeed" requires one to use the Governor::USERSPACE
     * governor.
     */
    bowl::MaybeError<bowl::CustomError> frequency(uint64_t frequency)
    {
        auto path = cpufreq_path() / "scaling_setspeed";

        return write_to_file(path, frequency / 1000);
    }

    /*
     * Gets a list of supported frequencies for the given Cpu.
     *
     * This is currently only implemented for the "acpi-cpufreq" and "cppc_cpufreq"
     * cpufreq drivers. For other drivers bowl::CustomError is returned.
     */
    bowl::Expected<std::vector<uint64_t>, bowl::CustomError> available_frequencies()
    {
        CHECK_ASSIGN(drv, cpufreq_driver());

        if (drv == "acpi-cpufreq")
        {
            return available_frequencies_acpi_cpufreq();
        }
        else if (drv == "cppc_cpufreq")
        {
            return available_frequencies_cppc_cpufreq();
        }

        return bowl::Unexpected(bowl::CustomError(
            fmt::format("Can not get available frequencies for cpufreq driver \"{}\"", drv)));
    }

    /*
     * Returns the current governor of the given cpu.
     *
     * Returns bowl::ErrnoError if "scaling_governor" could not be read
     * for some reason.
     */
    bowl::Expected<Governor, bowl::CustomError> governor()
    {
        CHECK_ASSIGN(gov, read_from_file<std::string>(cpufreq_path() / "scaling_governor"));

        return string_to_governor(gov);
    }

    /*
     * Sets the governor of the given Cpu to `gov`.
     *
     * Returns bowl::ErrnoError if the govenror could not be set.
     * This is most likely due to:
     * - Not having the rights to write "scaling_governor".
     * - `gov` not being a valid governor for the current cpufreq driver
     *
     */
    bowl::MaybeError<bowl::CustomError> governor(Governor gov)
    {
        auto path = cpufreq_path() / "scaling_governor";
        std::string gov_str = governor_to_string(gov);

        auto write_res = write_to_file(path, gov_str);
        if (!write_res.ok())
        {
            return write_res.unpack_error();
        }

        if (gov != governor().unpack_ok())
        {
            return bowl::CustomError(
                "Could not set governor to " + governor_to_string(gov) +
                ". This usually means that your cpufreq driver does not support " +
                governor_to_string(gov));
        }

        return {};
    }

    /*
     * Binds the currently executing thread to the given Cpu.
     *
     * Returns bowl::ErrnoError if sched_setaffinity fails.
     */
    bowl::MaybeError<bowl::ErrnoError> bind_this_thread_to()
    {
        cpu_set_t set;

        CPU_ZERO(&set);
        CPU_SET(cpu_, &set);

        if (sched_setaffinity(0, sizeof(set), &set) == -1)
        {
            return bowl::ErrnoError();
        }

        return {};
    }

    /*
     * Returns the current turbo state of the given Cpu.
     *
     * Returns bowl::CustomError if reading "cpufreq" / "boost" failed.
     *
     */
    static bowl::Expected<TurboState, bowl::CustomError> turbo()
    {
        CHECK_ASSIGN(turbo_int, read_from_file<uint64_t>(sysfs_path() / "cpufreq" / "boost"));

        return int_to_turbo_state(turbo_int);
    }

    /*
     * Sets the turbo state of the given Cpu.
     *
     * Returns bowl::CustomErrorif writing "cpufreq" / "boost" failed.
     */
    static bowl::MaybeError<bowl::CustomError> turbo(TurboState state)
    {
        return write_to_file(sysfs_path() / "cpufreq" / "boost", turbo_state_to_int(state));
    }

    /*
     * Returns a list of available governors for the current cpufreq driver
     *
     * Returns bowl::CustomError if reading "scaling_available_governors" failed.
     */
    bowl::Expected<std::vector<Governor>, bowl::CustomError> available_governors()
    {
        CHECK_ASSIGN(strings, read_from_file_list<std::string>(cpufreq_path() /
                                                               "scaling_available_governors"));

        std::vector<Governor> res;

        for (auto str : strings)
        {
            CHECK_ASSIGN(gov, string_to_governor(str));

            res.emplace_back(gov);
        }

        return res;
    }

    /*
     * Get a list of the Cpu's supported cstates and their
     * current status (enabled or disabled).
     *
     * The returned vector is sorted by the depth of the cstate, so
     * result[0] is the least deep cstate and
     * result[result.length() - 1] is the deepest cstate.
     *
     * Returns bowl::CustomError if reading
     * /sys/devices/system/cpu{cpuid}/cpuidle/state{stateid}/{name, disable}
     *
     * fails.
     */
    bowl::Expected<std::vector<std::pair<CState, CStateState>>, bowl::CustomError> get_cstates()
    {
        std::vector<std::pair<CState, CStateState>> res;

        unsigned int cstate_number = 0;
        std::filesystem::path cstate_path;

        for (int cstate_number = 0; std::filesystem::exists(
                 cstate_path = cpu_cpuidle_path() / fmt::format("state{}", cstate_number));
             cstate_number++)
        {
            CHECK_ASSIGN(name, read_from_file<std::string>(cstate_path / "name"));
            CHECK_ASSIGN(disable, read_from_file<uint64_t>(cstate_path / "disable"));

            CHECK_ASSIGN(cstate_state, cpuidle_disable_to_cstate_state(disable));

            res.emplace_back(std::pair<CState, CStateState>(CState(name), cstate_state));
        }
        return res;
    }

    /*
     * Sets the state of  `cstate' to `cstate_state` (enabled or disabled)
     *
     * This returns bowl::CustomError if `cstate` is not a known cstate or
     * writing /sys/devices/system/cpu{cpuid}/cpuidle/state{x}/disable fails.
     */
    bowl::MaybeError<bowl::CustomError> set_cstate(CState cstate, CStateState cstate_state)
    {
        CHECK_ASSIGN(cstate_path, get_cstate_path_for(cstate));

        return write_to_file(cstate_path / "disable",
                             cstate_state_to_cpuidle_disable(cstate_state));
    }

    /*
     * Converts `name` into a CState.
     *
     * Returns bowl::CustomError if `name` is not a known CState
     */
    bowl::Expected<CState, bowl::CustomError> cstate_from_str(std::string name)
    {
        CHECK_ASSIGN(cstates, get_cstates());

        for (auto cstate : cstates)
        {
            if (cstate.first.get_name() == name)
            {
                return std::move(cstate.first);
            }
        }

        return bowl::Unexpected(bowl::CustomError(fmt::format("Unknown cstate: {}", name)));
    }

    /*
     * In systems supporting ACPI CPPC, this returns the
     * frequency at the nominal_perf() setting.
     *
     * Returns bowl::CustomError if reading "nominal_freq" failed.
     */
    bowl::Expected<uint64_t, bowl::CustomError> nominal_freq()
    {
        CHECK_ASSIGN(mhz_freq, sysfs_cppc_read("nominal_freq"));

        return mhz_freq * 1000 * 1000;
    }

    /*
     * In systems supporting ACPI CPPC, this returns the
     * frequency at the lowest_perf() setting.
     *
     * Returns bowl::CustomError if reading "lowest_freq" failed.
     */
    bowl::Expected<uint64_t, bowl::CustomError> lowest_freq()
    {
        CHECK_ASSIGN(mhz_freq, sysfs_cppc_read("lowest_freq"));

        return mhz_freq * 1000 * 1000;
    }

    /*
     * ACPI CPPC aside:
     *
     * ACPI CPPC is an ACPI standard which allows for controlling the
     * performance of a processor along an abstract "perf" scale from
     * lowest_perf -> nominal_perf -> highest_perf.
     *
     * In practice (such as the definition used by the Linux kernel), the
     * perf scale is often not really that abstract, but represents equidistant
     * frequency steps.
     *
     * See available_frequencies_cppc_cpufreq() how one can calculate the
     * underlying frequencies from the ACPI CPPC information.
     *
     */

    /*
     * In systems supporting ACPI CPPC, this lowest possible "perf" setting.
     *
     * Returns bowl::CustomError if reading "lowest_perf" failed.
     */
    bowl::Expected<uint64_t, bowl::CustomError> lowest_perf()
    {
        return sysfs_cppc_read("lowest_perf");
    }

    /*
     * In systems supporting ACPI CPPC, the nominal "perf" setting.
     *
     * Returns bowl::CustomError if reading "nominal_perf" failed.
     */
    bowl::Expected<uint64_t, bowl::CustomError> nominal_perf()
    {
        return sysfs_cppc_read("nominal_perf");
    }

    /*
     * In systems supporting ACPI CPPC, the highest "perf" setting.
     *
     * Returns bowl::CustomError if reading "highest_perf" failed.
     */
    bowl::Expected<uint64_t, bowl::CustomError> highest_perf()
    {
        return sysfs_cppc_read("highest_perf");
    }

    /*
     * Returns the current cpufreq driver.
     *
     * Returns bowl::CustomError if reading "scaling_driver" failed.
     */
    bowl::Expected<std::string, bowl::CustomError> cpufreq_driver()
    {
        return read_from_file<std::string>(cpufreq_path() / "scaling_driver");
    }

    friend bool operator==(const Cpu& lhs, const Cpu& rhs)
    {
        return lhs.cpu_ == rhs.cpu_;
    }

    friend bool operator<(const Cpu& lhs, const Cpu& rhs)
    {
        return lhs.cpu_ < rhs.cpu_;
    }

    friend bool operator>(const Cpu& lhs, const Cpu& rhs)
    {
        return lhs.cpu_ > rhs.cpu_;
    }

private:
    /*
     * Reads the list of available frequencies for the "acpi-cpufreq"
     * cpufreq driver.
     *
     *
     * This is done by reading the "scaling_available_frequencies" sysfs file.
     *
     * Returns bowl::CustomError if reading "scaling_available_frequencies" failed.
     */
    bowl::Expected<std::vector<uint64_t>, bowl::CustomError> available_frequencies_acpi_cpufreq()
    {
        CHECK_ASSIGN(
            freqs, read_from_file_list<uint64_t>(cpufreq_path() / "scaling_available_frequencies"));

        std::vector<uint64_t> res;

        for (uint64_t i : freqs)
        {
            res.emplace_back(i * 1000);
        }

        return res;
    }

    /*
     * Reads the list of available frequencies for the "cppc_cpufreq" driver.
     *
     * This follows the calculations done in linux/drivers/acpi/cppc_acpi.c
     * Function "cppc_perf_to_khz".
     *
     * Returns bowl::CustomError if reading any of the CPPC sysfs files failed.
     */
    bowl::Expected<std::vector<uint64_t>, bowl::CustomError> available_frequencies_cppc_cpufreq()
    {
        uint64_t mul;
        uint64_t div;

        CHECK_ASSIGN(lowest_freq, lowest_freq());
        CHECK_ASSIGN(nominal_freq, nominal_freq());

        CHECK_ASSIGN(lowest_perf, lowest_perf());
        CHECK_ASSIGN(nominal_perf, nominal_perf());
        CHECK_ASSIGN(highest_perf, highest_perf());

        if (lowest_freq == nominal_freq)
        {
            mul = nominal_freq;
            div = nominal_perf;
        }
        else
        {
            mul = nominal_freq - lowest_freq;
            div = nominal_perf - lowest_perf;
        }

        uint64_t offset = nominal_freq - ((nominal_perf * mul) / div);

        std::vector<uint64_t> freqs;
        for (uint64_t perf = lowest_perf; perf <= highest_perf; perf++)
        {
            freqs.emplace_back(offset + ((perf * mul) / div));
        }
        return freqs;
    }

    /*
     * /sys/devices/system/cpu/cpu{cpu_}
     */
    std::filesystem::path cpu_sysfs_path()
    {
        return sysfs_path() / fmt::format("cpu{}", cpu_);
    }

    /*
     * /sys/devices/system/cpu/cpu{cpu_}/cpufreq
     */
    std::filesystem::path cpufreq_path()
    {
        return cpu_sysfs_path() / "cpufreq";
    }

    /*
     * /sys/devices/system/cpu/cpu{cpu_}/acpi_cppc
     */
    std::filesystem::path acpi_cppc_path()
    {
        return cpu_sysfs_path() / "acpi_cppc";
    }

    /*
     * /sys/devices/system/cpu/cpu{cpu_}/cpuidle
     */
    std::filesystem::path cpu_cpuidle_path()
    {
        return cpu_sysfs_path() / "cpuidle";
    }

    static std::filesystem::path sysfs_path()
    {
        return "/sys/devices/system/cpu";
    }

    bowl::Expected<uint64_t, bowl::CustomError> sysfs_cppc_read(std::string file)
    {
        return read_from_file<uint64_t>(acpi_cppc_path() / file);
    }

    /*
     * For CState `state`, get its path in the sysfs.
     *
     * Returns bowl::CustomError if no such path can be found.
     */
    bowl::Expected<std::filesystem::path, bowl::CustomError> get_cstate_path_for(CState state)
    {
        std::filesystem::path cstate_path;
        for (int cstate_number = 0; std::filesystem::exists(
                 cstate_path = cpu_cpuidle_path() / fmt::format("state{}", cstate_number));
             cstate_number++)
        {

            CHECK_ASSIGN(name, read_from_file<std::string>(cstate_path / "name"));

            if (state.get_name() == name)
            {
                return cstate_path;
            }
        }

        return bowl::Unexpected(bowl::CustomError(fmt::format(
            "Can not get {} path for cstate: {}", cpu_cpuidle_path().string(), state.get_name())));
    }

    explicit Cpu(int cpuid) : cpu_(cpuid)
    {
    }

    int cpu_;
};

} // namespace penguinxx
