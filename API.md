# Python API Reference

`sorting_bench` — sorting algorithm benchmarking framework.

```python
import sorting_bench as sb
```

---

## Enums

### `DataType`

| Member | String |
|--------|--------|
| `sb.DataType.RANDOM` | `"random"` |
| `sb.DataType.SORTED` | `"sorted"` |
| `sb.DataType.REVERSED` | `"reversed"` |
| `sb.DataType.NEARLY_SORTED` | `"nearly_sorted"` |
| `sb.DataType.DUPLICATES` | `"duplicates"` |
| `sb.DataType.CONSTANT` | `"constant"` |

---

## Data Structures

### `SortStats` — operation counters

```python
stats = sb.SortStats()
stats.compare_count   # long long, default 0
stats.move_count      # long long  (each swap adds 2 moves)
stats.swap_count      # long long  (subset of move_count)
stats.key_op_count()  # compare_count + move_count
                      # already covers swaps — see note below
```

> **Note on `key_op_count`**: `TraceArray.swap()` internally records 1 swap + 2 moves
> (two element assignments). Therefore `move_count` already includes the movement
> costs of swaps, and `key_op_count = compare_count + move_count` is the complete
> picture. Do **not** add `swap_count` on top — that would triple-count swap work.

### `BenchmarkConfig` — benchmark configuration

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `dataset_id` | `str` | `"default_dataset"` | Identifier for the dataset |
| `input_id` | `str` | `"input_0"` | Identifier for this input |
| `data_type` | `str` | `"unknown"` | Label for the data type |
| `repeat_times` | `int` | `1` | Number of repeat runs for averaging |
| `timeout_ms` | `int` | `0` | Timeout in ms per run (0 = disabled) |

```python
config = sb.BenchmarkConfig()
config.repeat_times = 5
config.timeout_ms = 500
```

### `BenchmarkResult` — per-algorithm result

All fields are read/write. Populated by `BenchmarkRunner.run()`.

| Field | Type | Description |
|-------|------|-------------|
| `dataset_id` | `str` | |
| `input_id` | `str` | |
| `data_type` | `str` | |
| `data_size` | `int` | |
| `algorithm_name` | `str` | |
| `elapsed_ms` | `float` | Mean across repeats |
| `median_elapsed_ms` | `float` | Median across repeats |
| `stddev_elapsed_ms` | `float` | Sample stddev |
| `compare_count` | `int` | Mean per repeat |
| `move_count` | `int` | Mean per repeat |
| `swap_count` | `int` | Mean per repeat |
| `key_op_count` | `int` | compare_count + move_count (swap movement included via moves) |
| `sorted_correctly` | `bool` | |
| `timed_out` | `bool` | |
| `error_message` | `str` | |
| `time_rank` | `int` | Set by `ResultRanker.rank()` |
| `operation_rank` | `int` | Set by `ResultRanker.rank()` |
| `overall_score` | `float` | Set by `ResultRanker.rank()` |
| `overall_rank` | `int` | Set by `ResultRanker.rank()` |

### Dataset types

**`DatasetInput`** — a single data vector:

| Field | Type |
|-------|------|
| `id` | `str` |
| `values` | `list[int]` |

**`DatasetSpec`** — specification for generating a dataset:

| Field | Type | Default |
|-------|------|---------|
| `id` | `str` | `""` |
| `data_type` | `DataType` | `RANDOM` |
| `input_size` | `int` | `0` |
| `input_count` | `int` | `1` |
| `min_value` | `int` | `0` |
| `max_value` | `int` | `10000` |

Method: `effective_id()` — returns `id` if non-empty, otherwise `"<type>_<size>"`.

**`Dataset`** — a named collection of inputs:

| Field | Type |
|-------|------|
| `spec` | `DatasetSpec` |
| `inputs` | `list[DatasetInput]` |

Methods: `id()`, `data_type_name()`, `input_count()`.

**`DatasetGridSpec`** — Cartesian product of types × sizes:

| Field | Type | Default |
|-------|------|---------|
| `data_types` | `list[DataType]` | |
| `input_sizes` | `list[int]` | |
| `input_count_per_dataset` | `int` | `1` |
| `min_value` | `int` | `0` |
| `max_value` | `int` | `10000` |
| `id_prefix` | `str` | `""` |

**`DatasetMixPart`** — a component in a mixed dataset:

| Field | Type |
|-------|------|
| `data_type` | `DataType` |
| `ratio` | `float` |

**`DatasetMixSpec`** — ratio-based mixed dataset:

| Field | Type | Default |
|-------|------|---------|
| `input_size` | `int` | `0` |
| `total_input_count` | `int` | `0` |
| `parts` | `list[DatasetMixPart]` | |
| `min_value` | `int` | `0` |
| `max_value` | `int` | `10000` |
| `id_prefix` | `str` | `""` |

**`DatasetSuite`** — collection of datasets:

| Field | Type |
|-------|------|
| `datasets` | `list[Dataset]` |

Methods: `dataset_count()`, `input_row_count()`.

### Aggregated results

**`BenchmarkDatasetResult`:**

| Field | Type |
|-------|------|
| `dataset` | `Dataset` |
| `ranked_results_by_input` | `list[list[BenchmarkResult]]` |

Methods: `input_row_count()`, `flatten_results()`.

**`BenchmarkSuiteResult`:**

| Field | Type |
|-------|------|
| `dataset_results` | `list[BenchmarkDatasetResult]` |

Methods: `dataset_count()`, `input_row_count()`, `flatten_results()`.

---

## Classes

### `TraceArray` — instrumented array for sorting

The array that sorting algorithms operate on. Automatically records comparisons, moves, and swaps. Supports cancellation via timeout.

```python
# Usually you don't construct this directly — BenchmarkRunner creates it
arr.size()                               # int
len(arr)                                 # same as size()
arr.empty()                              # bool
arr.get(index)                           # int at index
arr[index]                               # same as get()
arr.set(index, value)                    # set value at index
arr[index] = value                       # same as set()
arr.less(left, right)                    # bool: arr[left] < arr[right] (records compare)
arr.less_value(index, value)             # bool: arr[index] < value
arr.greater(left, right)                 # bool: arr[left] > arr[right]
arr.greater_value(index, value)          # bool: arr[index] > value
arr.swap(left, right)                    # swap arr[left] and arr[right] (records swap)
arr.values()                             # read-only view of underlying list[int]
```

### `Sorter` — abstract base for sorting algorithms

Subclass this to implement a custom algorithm in Python:

```python
class MySort(sb.Sorter):
    def name(self):
        return "MySort"

    def sort(self, arr):
        # arr is a TraceArray — use its API, not raw list ops
        # Goal: ascending order. If arr[j] > arr[j+1], swap them
        n = len(arr)
        for i in range(n):
            for j in range(n - i - 1):
                if arr.greater(j, j + 1):
                    arr.swap(j, j + 1)
```

Built-in concrete sorters: `sb.QuickSort()`, `sb.SlowSort()`.

### `BenchmarkRunner` — the benchmark engine

```python
runner = sb.BenchmarkRunner()
runner.add_sorter(sorter)          # register a Sorter (C++ or Python subclass)
runner.clear_sorters()             # remove all registered sorters

# Run on a single input list
results = runner.run(raw_data, config)
# raw_data: list[int]
# config: BenchmarkConfig
# returns: list[BenchmarkResult]

# Run on a Dataset
dataset_result = runner.run_dataset(dataset, config=BenchmarkConfig())
# returns: BenchmarkDatasetResult

# Run on a DatasetSuite
suite_result = runner.run_suite(suite, config=BenchmarkConfig())
# returns: BenchmarkSuiteResult
```

Key behaviors:
- **`run()`**: copies `raw_data` into a fresh `TraceArray` for each algorithm × repeat, times execution via high-resolution clock, and checks correctness (output must be non-decreasing).
- **Timeout**: if `config.timeout_ms > 0`, a timer thread sets a cancellation flag. `TraceArray` operations check this flag and throw `SortCancelledError` if fired, aborting the sort gracefully.
- **`run_dataset()` / `run_suite()`** automatically rank results for each input via `ResultRanker.rank()`.

---

## Static Utility Classes

### `DataGenerator` — test data generation

```python
sb.DataGenerator.random_data(size, min_val, max_val)          # list[int]
sb.DataGenerator.sorted_data(size, start_val=0, step=1)       # list[int]
sb.DataGenerator.reversed_data(size, start_val=-1)            # list[int]
sb.DataGenerator.nearly_sorted_data(size, start_val=0, step=1, swap_ratio=0.05)
sb.DataGenerator.duplicate_data(size, min_val, max_val, distinct_count=5)
sb.DataGenerator.constant_data(size, value=42)                # list[int]

# Generate by string or enum
sb.DataGenerator.generate(data_type, size, min_val=0, max_val=10000)
# data_type: "random" | "sorted" | "reversed" | "nearly_sorted" | "duplicates" | "constant"
sb.DataGenerator.generate_by_type(sb.DataType.SORTED, size)

# Generate Dataset / DatasetSuite
sb.DataGenerator.generate_dataset(data_type, input_count, input_size, min_val, max_val)
sb.DataGenerator.generate_dataset_from_spec(spec)             # Dataset from DatasetSpec
sb.DataGenerator.generate_suite(grid_spec)                    # DatasetSuite from DatasetGridSpec
sb.DataGenerator.generate_suite_from_mix(mix_spec)            # DatasetSuite from DatasetMixSpec
```

### `DataReader` / `DataWriter` — file I/O

```python
sb.DataWriter.write_binary(data, file_path)   # bool
sb.DataWriter.write_text(data, file_path)     # bool
sb.DataReader.read_binary(file_path)          # list[int]
sb.DataReader.read_text(file_path)            # list[int]
```

Binary format: 4-byte magic `SDAT` + 8-byte count + int32 array.

### `ResultRanker` — competition ranking

```python
sb.ResultRanker.rank(results)
# Mutates results in-place: time_rank, operation_rank, overall_score, overall_rank
# Ranking style: 1 2 2 4 (ties share the rank, next rank skips)
# Failed sorters (sorted_correctly=False) get overall_score = max and sink to bottom
```

### `ReportWriter` — console & CSV output

```python
sb.ReportWriter.write_console(results)            # pretty-printed table to stdout
sb.ReportWriter.write_suite_console(suite_result)
sb.ReportWriter.write_csv(results, file_path)     # bool
sb.ReportWriter.write_suite_csv(suite_result, file_path)
```

### `TrainingExport` — ML training data

```python
s = sb.TrainingExport.serialize_data(raw_data)    # space-separated string

sb.TrainingExport.write_training_csv(raw_data, config, ranked_results, file_path)
sb.TrainingExport.write_training_dataset_csv(inputs, config, ranked_results_by_input, file_path)
sb.TrainingExport.write_training_dataset_csv_from_result(dataset_result, file_path)
sb.TrainingExport.write_training_suite_csv(suite_result, file_path)
```

CSV columns: `dataset_id`, `input_id`, `data_type`, `size`, `data`, `best_algorithm`, `best_time_ms`, `best_key_ops`, `best_moves`, `best_swaps`, `best_correct`, `all_algorithms`, `all_times_ms`, `all_key_ops`, `all_moves`, `all_swaps`.

---

## Free Functions

```python
sb.data_type_name(DataType)           # str: "random", "sorted", ...
sb.default_dataset_id(DataType, size)  # str: "sorted_1000", etc.
```

---

## Exception

```python
sb.SortCancelledError  # raised when a TraceArray operation detects timeout
```

**Normal usage** — `BenchmarkRunner.run()` / `run_dataset()` / `run_suite()` catch
`SortCancelledError` internally. A timed-out algorithm **does not** crash the
benchmark; instead it produces:

```python
result.timed_out = True
result.sorted_correctly = False
result.error_message = "timeout"
```

All other sorters continue, and the timed-out sorter sinks to the bottom in rankings.
**You do not need to catch `SortCancelledError` around `runner.run()`.**

**Advanced usage** — if you call `TraceArray` operations directly (outside of
`BenchmarkRunner`), the timeout cancellation flag may throw:

```python
try:
    my_sorter.sort(trace_array)
except sb.SortCancelledError:
    print("Sort was cancelled by timeout")
```

This only applies when you construct and drive a `TraceArray` with a cancellation
flag yourself — rare with normal API usage.

---

## End-to-End Example

```python
import sorting_bench as sb

# 1. Generate test data
data = sb.DataGenerator.random_data(1000, 0, 10000)

# 2. Create runner with built-in and custom sorters
runner = sb.BenchmarkRunner()
runner.add_sorter(sb.QuickSort())
runner.add_sorter(sb.SlowSort())

# 3. Define a custom Python sorter
class InsertionSort(sb.Sorter):
    def name(self): return "PyInsertionSort"
    def sort(self, arr):
        # Ascending order: shift elements greater than key to the right
        for i in range(1, len(arr)):
            key = arr.get(i)
            j = i - 1
            while j >= 0 and arr.greater_value(j, key):
                arr.set(j + 1, arr.get(j))
                j -= 1
            arr.set(j + 1, key)

runner.add_sorter(InsertionSort())

# 4. Run benchmark
config = sb.BenchmarkConfig()
config.repeat_times = 5
config.timeout_ms = 500
results = runner.run(data, config)

# 5. Rank and report
sb.ResultRanker.rank(results)
sb.ReportWriter.write_console(results)
sb.ReportWriter.write_csv(results, "report.csv")

# 6. Export training data
sb.TrainingExport.write_training_csv(data, config, results, "training.csv")

# 7. Run a full suite
grid = sb.DatasetGridSpec()
grid.data_types = [sb.DataType.RANDOM, sb.DataType.SORTED, sb.DataType.REVERSED]
grid.input_sizes = [100, 500, 1000]
grid.input_count_per_dataset = 3

suite = sb.DataGenerator.generate_suite(grid)
suite_result = runner.run_suite(suite, config)
sb.ReportWriter.write_suite_console(suite_result)
sb.TrainingExport.write_training_suite_csv(suite_result, "suite_training.csv")
```
