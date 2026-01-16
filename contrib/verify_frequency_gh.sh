#!/usr/bin/env bash
#
# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: (c) 2025 Christian von Elm <christian.von_elm@tu-dresden.de

##################################
# VERIFY FREQUENCY SETTING CODE  #
# VERSION FOR NVIDIA GRACEHOPPER #
##################################
#
# *Gracehopper note*: GraceHopper does not have
# the cycles event, instead cpu_cycles is used.
# This also means that the parsing code needed to be
# thrown around a bit.
#
# This shell script verifies whether:
#
# $ ./penguinxx-cpu frequency set $FREQUENCY
#
# ... sets the correct frequency.
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

$PENGUINXX_CPU turbo set OFF

# For every available frequency.
$PENGUINXX_CPU frequency list | while read -r freq; do
    $PENGUINXX_CPU frequency set $freq

    # Measure effective processor frequency by executing the busy_wait kernel
    # while recording executed cycles.
    perf stat -e cpu_cycles -e task-clock -j -- $KERNEL_RUNNER busy_wait $(${PENGUINXX_CPU_SET} all) 10 2>foobar.json

    TIME_MSEC=$(cat foobar.json | jq -r 'select(.event == "task-clock") | ."counter-value"')
    CYCLES=$(cat foobar.json | jq -r 'select(.event == "cpu_cycles") | ."counter-value"')

    OUTHZ=$(echo "print(int($CYCLES/($TIME_MSEC/1000)))" | python3)
    echo $OUTHZ
    RATIO=$(echo "print(int($OUTHZ / $freq * 100))" | python3)

    if [[ $RATIO -gt 95 && $RATIO -lt 105 ]]
    then
        echo "Setting cpufreq=$freq Hz is within +-5%!: $RATIO%";
    else
        echo "Read value for cycles out of +-5% set frequency: $RATIO%"
        exit 1
    fi
done
