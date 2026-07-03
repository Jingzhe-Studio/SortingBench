# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

Sorting Bench is a C++17 benchmarking framework for sorting algorithms. It measures both wall-clock time and operation counts (comparisons, moves, swaps), ranks algorithms, and exports results as CSV reports and ML training data.

## Build

This is a Visual Studio 2022+ project (toolset v145). Open `sorting-bench/SortingBench.sln` in Visual Studio, or build from the command line:

```bash
# From the sorting-bench/ directory
msbuild SortingBench.sln /p:Configuration=Release /p:Platform=x64
```

The executable is a console application — the entry point is `examples/cli_demo.cpp`. There are no tests or other build targets; `cli_demo.cpp` doubles as a smoke-test harness.

## Architecture

### Strategy pattern for algorithms

All sorting algorithms implement the `Sorter` interface (`src/contract/Sorter.h`):

```cpp
class Sorter {
    virtual std::string name() const = 0;
    virtual void sort(TraceArray& data) = 0;
};
```

Concrete implementations live in `src/algorithms/`: BubbleSort, SelectionSort, InsertionSort, ShellSort, MergeSort, QuickSort. **Only QuickSort is currently implemented** — the other five are empty stubs (zero-byte .cpp/.h files). To add a new algorithm, subclass `Sorter`, implement `name()` and `sort()`, then register it with `BenchmarkRunner::addSorter()`.

### Instrumented array (`TraceArray`)

`src/trace/TraceArray.h` wraps `std::vector<int>` and transparently counts comparisons, moves, and swaps into a `SortStats` struct. Sorting algorithms operate on `TraceArray` instead of raw vectors — this is how the framework captures operation counts without modifying the algorithm logic. Use `less()`, `greater()`, `swap()`, `get()`, `set()` instead of raw vector access.

### Data generation pipeline

`DataGenerator` (`src/data/`) produces test vectors of six data types via the `DataType` enum: `Random`, `Sorted`, `Reversed`, `NearlySorted`, `Duplicates`, `Constant`. It can:

- Generate individual vectors (`randomData()`, `sortedData()`, etc.)
- Dispatch by string name (`generate("nearly_sorted", N)`)
- Build `Dataset` objects (multiple inputs sharing a spec) via `generateDataset(DatasetSpec)`
- Build `DatasetSuite` (multiple datasets across a grid of types × sizes) via `generateSuite(DatasetGridSpec)`, or ratio-based mixed suites via `generateSuite(DatasetMixSpec)`

### Benchmarking flow

1. **Configure**: `BenchmarkConfig` specifies `datasetId`, `dataType`, `repeatTimes` (for averaging over multiple runs)
2. **Run**: `BenchmarkRunner` iterates registered sorters against input data. For each sorter, it creates a fresh `TraceArray` copy (preserving the original), times execution via `std::chrono::high_resolution_clock`, and collects `SortStats`
3. **Aggregate**: Multiple repeats produce mean/median/stddev for elapsed time and mean values for operation counts. Correctness is verified by checking the array is sorted after each run
4. **Rank**: `ResultRanker::rank()` computes competition-style ranks (`1 2 2 4` for ties) for time (`timeRank`) and operations (`operationRank`), then an `overallScore` as the mean of both ranks. Failed sorters get `overallScore = max` and sink to the bottom
5. **Report**: `ReportWriter` outputs formatted console tables or CSV. `TrainingExport` writes ML-oriented CSV rows where each row contains the serialized input data, the best algorithm (label), and per-algorithm metrics

### I/O and serialization

`DataWriter` / `DataReader` support binary (4-byte magic + count + int32 array) and plain-text (one int per line) formats for persisting input data.

### Key data structures

- `SortStats` — raw counters: `compareCount`, `moveCount`, `swapCount`, computed `keyOpCount()` (compares + moves)
- `BenchmarkResult` — per-algorithm, per-input result: timing stats, operation counts, correctness flag, and rank fields
- `Dataset` — a `DatasetSpec` + multiple `DatasetInput` vectors
- `DatasetSuite` — collection of `Dataset` objects
- `BenchmarkDatasetResult` / `BenchmarkSuiteResult` — nested result containers matching the Dataset hierarchy

### Directory map

```
sorting-bench/
├── include/sorting_benchmark.hpp   # Public umbrella header
├── examples/cli_demo.cpp           # Main executable & smoke test
├── src/
│   ├── contract/Sorter.h           # Abstract sorter interface
│   ├── algorithms/                 # Sorting algorithm implementations
│   ├── trace/TraceArray.h          # Instrumented vector with op counting
│   ├── data/                       # Data generation, datasets, serialization
│   ├── benchmark/                  # Runner, config, result structs
│   └── postprocess/                # Ranking, console/CSV reporting, training export
└── SortingBench.sln / .vcxproj     # VS2022 build files
```
