# 🐧++ No-throw, Type-Safe C++ wrappers for Linux interfaces

This library provides checked, type-safe wrappers to Linux interfaces commonly used by me.

This library is implemented in a non-throw fashion using the [bowl](https://github.com/cvonelm/bowl) error types.

## Dependencies

- A C++17 compiler
- Catch2 (If PENGUINXX_TESTS=ON)
- libnuma

## Building

This repository makes use of submodules.

Be sure to clone them with

```
$ git submodule update --init --recursive
```

Then

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Using penguinxx in your own projects

No workflow for installing penguinxx exists currently.

The easiest way to make use of penguinxx is too add

```cmake
add_subdirectory(lib/penguinxx)
```

to your CMakeLists.txt file.

## Currently available modules

**Penguinxx::cpu** library
- `penguinxx::Cpu`
  - Get/set governor/frequency/turbo state
  - List all available governors/frequencies
  - [...]
- `penguinxx::CpuSet`
  - Create CPU sets from strings ("0-5")
  - Create unions of CpuSets
  - Print CpuSets to strings
- `penguinxx::CpuTopology`
  - Get a list of all CPUs in the current system
- `penguinxx::NUMANodeSet`
  - Create NUMA node sets
  - Bind currently executing thread to NUMANodeSet

**Penguinxx::core** library
- `penguinxx::Barrier`
  - Create and wait on a Barrier
- `penguinxx::Pthread`
  - Interface with pthreads

## Tools

If `-DPENGUINXX_TOOLS=ON` is enabled during build, then the penguinxx tools are build.

These demonstrate the penguinxx capabilities as well as providing them on the command-line.

Currently two tools are available:

**penguinxx-cpu** -- Set CPU attributes

- `penguinxx-cpu governor set/get` - Sets/Gets CPU governor
- `penguinxx-cpu frequency set/get/list` - Sets/Gets CPU frequency, list available frequencies
- `penguinxx-cpu turbo set/get` - Sets/Gets CPU turbo state

**penguinxx-cpu-set** -- Generate CPU Sets
- `penguinxx-cpu-set staircase` - One per line, generate a staircase pattern, e.g.:
  ```
  0
  0,1
  0,1,2
  ...
  ```
- `penguinxx-cpu-set all` - Print a CPU set containing all CPUs

## Documentation

The penguinxx header files are extensively documented. Outside of that,
the `tools/` provide hints on the usage of this library.

## License

This project is licensed under the terms of the MIT License.
For more information see [LICENSE](LICENSE)
