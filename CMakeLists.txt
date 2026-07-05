cmake_minimum_required(VERSION 3.18)
project(sorting-bench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Dependencies — pybind11 (shipped in-tree, v2.12.0)
# ============================================================
# pybind11 is committed directly in third_party/pybind11.
# No network needed at configure time — just clone the repo and build.
add_subdirectory(third_party/pybind11)
# Prevent pybind11 from installing itself as part of our build
set(PYBIND11_INSTALL OFF CACHE BOOL "" FORCE)

# ============================================================
# Common source files (all .cpp except cli_demo and empty stubs)
# ============================================================
set(SORTING_BENCH_SOURCES
    sorting-bench/src/algorithms/BubbleSort.cpp
    sorting-bench/src/algorithms/InsertionSort.cpp
    sorting-bench/src/algorithms/MergeSort.cpp
    sorting-bench/src/algorithms/QuickSort.cpp
    sorting-bench/src/algorithms/SelectionSort.cpp
    sorting-bench/src/algorithms/ShellSort.cpp
    sorting-bench/src/algorithms/SlowSort.cpp
    sorting-bench/src/benchmark/BenchmarkRunner.cpp
    sorting-bench/src/data/DataGenerator.cpp
    sorting-bench/src/data/DataReader.cpp
    sorting-bench/src/data/DataWriter.cpp
    sorting-bench/src/postprocess/ReportWriter.cpp
    sorting-bench/src/postprocess/ResultRanker.cpp
    sorting-bench/src/postprocess/TrainingExport.cpp
)

set(SORTING_BENCH_HEADERS
    sorting-bench/include/sorting_benchmark.hpp
    sorting-bench/src/algorithms/BubbleSort.h
    sorting-bench/src/algorithms/InsertionSort.h
    sorting-bench/src/algorithms/MergeSort.h
    sorting-bench/src/algorithms/QuickSort.h
    sorting-bench/src/algorithms/SelectionSort.h
    sorting-bench/src/algorithms/ShellSort.h
    sorting-bench/src/algorithms/SlowSort.h
    sorting-bench/src/benchmark/BenchmarkConfig.h
    sorting-bench/src/benchmark/BenchmarkDatasetResult.h
    sorting-bench/src/benchmark/BenchmarkResult.h
    sorting-bench/src/benchmark/BenchmarkRunner.h
    sorting-bench/src/benchmark/SortStats.h
    sorting-bench/src/contract/SortCancelledException.h
    sorting-bench/src/contract/Sorter.h
    sorting-bench/src/data/DataGenerator.h
    sorting-bench/src/data/DataReader.h
    sorting-bench/src/data/Dataset.h
    sorting-bench/src/data/DataWriter.h
    sorting-bench/src/postprocess/ReportWriter.h
    sorting-bench/src/postprocess/ResultRanker.h
    sorting-bench/src/postprocess/TrainingExport.h
    sorting-bench/src/trace/TraceArray.h
)

# ============================================================
# Target 1 — Static library (core framework, no main)
# ============================================================
add_library(sorting_bench_core STATIC ${SORTING_BENCH_SOURCES} ${SORTING_BENCH_HEADERS})
target_include_directories(sorting_bench_core PUBLIC
    sorting-bench/include
    sorting-bench/src
)

if(MSVC)
    target_compile_options(sorting_bench_core PRIVATE /utf-8 /W3 /EHsc)
    # Disable minmax macros from windows.h (defensive)
    target_compile_definitions(sorting_bench_core PRIVATE NOMINMAX)
endif()

# ============================================================
# Target 2 — Console executable (cli_demo.cpp)
# ============================================================
add_executable(cli_demo sorting-bench/examples/cli_demo.cpp)
target_link_libraries(cli_demo PRIVATE sorting_bench_core)

if(MSVC)
    target_compile_options(cli_demo PRIVATE /W3 /EHsc)
    target_compile_definitions(cli_demo PRIVATE NOMINMAX)
endif()

# ============================================================
# Target 3 — pybind11 Python extension module
# ============================================================
pybind11_add_module(sorting_bench sorting-bench/src/pybind/bindings.cpp)
target_link_libraries(sorting_bench PRIVATE sorting_bench_core)

if(MSVC)
    target_compile_options(sorting_bench PRIVATE /W3 /EHsc)
    target_compile_definitions(sorting_bench PRIVATE NOMINMAX)
endif()
