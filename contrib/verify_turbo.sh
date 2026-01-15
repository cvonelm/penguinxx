#!/usr/bin/env bash
#
# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

##############################
# VERIFY TURBO SETTING CODE  #
##############################
#
# This shell script verifies whether:
#
# $ ./penguinxx-cpu turbo set ON/OFF
#
# ... correctly enables/disables turbo frequencies.
#
# This might lead to spurious false negatives as there is afaik
# no way to detect simply whether turbo frequencies exist or not and
# architectures (like Ampere Altra) exist, that still have the "turbo"
# knob, but no turbo frequencies.
#
# No -o pipefail. This leads to spurious failures because head -n1 loves
# to SIGPIPE
set -eu

# Put your paths to penguinxx-cpu, penguinxx-cpu-set and roco2_kernels/kernel_runner
# here.
PENGUINXX_TOOLDIR=../build/tools
PENGUINXX_CPU=${PENGUINXX_TOOLDIR}/cpu/penguinxx-cpu
PENGUINXX_CPU_SET=${PENGUINXX_TOOLDIR}/cpu_set/penguinxx-cpu-set

KERNEL_RUNNER=../../roco2_kernels/build/kernel_runner

if [[ ! -e $KERNEL_RUNNER ]] ; then
    echo "Set \$KERNEL_RUNNER to a valid path to roco2_kernels/kernel_runner!"
    exit 1
fi

if [[ ! -e $PENGUINXX_CPU ]] ; then
    echo "Set \$PENGUINXX_CPU to a valid path to a penguinxx-cpu executable!";
    exit 1
fi

if [[ ! -e $PENGUINXX_CPU_SET ]] ; then
    echo "Set \$PENGUINXX_CPU_SET to a valid path to a penguinxx-cpu-set executable!";
    exit 1
fi

HIGHEST_FREQUENCY=$($PENGUINXX_CPU frequency list | sort -n -r | head -n 1)

$PENGUINXX_CPU frequency set $HIGHEST_FREQUENCY

$PENGUINXX_CPU turbo set ON

# Having one core busy_wait for 10 seconds should surely get you into turbo frequencies
TURBO_GHZ=$(perf stat -e cycles -C 0 -e task-clock -x " " -- $KERNEL_RUNNER busy_wait 0 10 2>&1   | head -n 1 | awk '{print $5}')

echo "Turbo: $TURBO_GHZ GHz"

$PENGUINXX_CPU turbo set OFF

NOMINAL_GHZ=$(perf stat -e cycles -C 0 -e task-clock -x " " -- $KERNEL_RUNNER busy_wait 0 10 2>&1   | head -n 1 | awk '{print $5}')

echo "Nominal: $NOMINAL_GHZ GHz"

RATIO=$(echo "print(int($TURBO_GHZ/$NOMINAL_GHZ*100))" | python3)

if [[ $RATIO -gt 105 ]]
then
    echo "Enabling and disabling turbo works! turbo frequency is $RATIO% higher than nominal";
else
    echo "Enabling and disabling turbo does not work: turbo frequency is $RATIO% of nominal!"
    echo "Does your CPU architecture have turbo frequencies?"
    exit 1;
fi
