# Build Guide (C++ Developers)

## Build Systems

This project supports two build systems:

| System | Targets | File |
|--------|---------|------|
| **CMake** (new) | `sorting_bench_core`, `cli_demo`, `sorting_bench` (Python module) | `CMakeLists.txt` |
| **MSBuild** (legacy) | `cli_demo.exe` only | `sorting-bench/SortingBench.sln` |

The MSBuild `.vcxproj` is preserved as-is. **For the Python module, you must use CMake.**

---

## CMake Build

### Prerequisites

- Visual Studio 2022+ (Community / Professional / Enterprise) with "Desktop development with C++" workload
- CMake >= 3.18 (`pip install cmake` if not installed)
- **Python 3.13.x** — `.pyd` 与 Python 版本绑定，必须用 3.13
- Git (required by FetchContent to clone pybind11)

### Quick Start

```bash
# Open "Developer PowerShell for VS 2022" or ensure cl.exe is on PATH

# 1. Configure — point to your Python installation
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release ^
  -DPYTHON_EXECUTABLE="C:/path/to/python.exe" ^
  -DPYTHON_LIBRARY="C:/path/to/libs/python313.lib" ^
  -DPYTHON_INCLUDE_DIR="C:/path/to/include"

# If Python is discoverable on PATH, you can simplify:
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 2. Build C++ executable only
cmake --build build --target cli_demo --config Release

# 3. Build Python module only
cmake --build build --target sorting_bench --config Release

# 4. Build everything
cmake --build build --config Release
```

### Find Your Python Paths

```bash
python -c "import sys; print(sys.executable)"
python -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"
python -c "import sysconfig; print(sysconfig.get_path('include'))"
```

Typical conda paths:
```
PYTHON_EXECUTABLE = C:/Users/<user>/miniconda3/python.exe
PYTHON_LIBRARY    = C:/Users/<user>/miniconda3/libs/python313.lib
PYTHON_INCLUDE_DIR = C:/Users/<user>/miniconda3/include
```

### Build Outputs

| Target | Output |
|--------|--------|
| `cli_demo` | `build/Release/cli_demo.exe` |
| `sorting_bench` | `build/Release/sorting_bench.cp313-win_amd64.pyd` |
| `sorting_bench_core` | `build/Release/sorting_bench_core.lib` |

### Test the Python module

```bash
cd build/Release
python -c "import sorting_bench; print(dir(sorting_bench))"
```

### Adding a New C++ Algorithm

1. Implement your sorter class inheriting from `Sorter` in `src/algorithms/`
2. Add `.h` and `.cpp` to `SORTING_BENCH_SOURCES` and `SORTING_BENCH_HEADERS` in `CMakeLists.txt`
3. Add binding code in `src/pybind/bindings.cpp` (see existing `QuickSort` binding as template)
4. Rebuild

---

## MSBuild Build (Legacy)

Open `sorting-bench/SortingBench.sln` in Visual Studio, select Release x64, and build. This produces only `cli_demo.exe` — no Python module.
