# 模块接口约定

> 本文档全部基于项目当前代码，不新增任何约定。各成员按此文档对齐即可并行开工。

---

## 1. 算法类名（A → B、C、D）

A 补齐后，`sorting_bench` 提供以下 7 个排序器（已有 QuickSort 和 SlowSort，新增 5 个）：

```python
import sorting_bench as sb

sb.QuickSort()       # 已有
sb.SlowSort()        # 已有（超时测试用，benchmark 不用它）
sb.BubbleSort()      # A 新增
sb.SelectionSort()   # A 新增
sb.InsertionSort()   # A 新增
sb.ShellSort()       # A 新增
sb.MergeSort()       # A 新增
```

每个排序器都继承 `sb.Sorter`，构造方式统一为无参构造。

---

## 2. BenchmarkRunner（B、C、D 共用）

```python
runner = sb.BenchmarkRunner()
runner.add_sorter(sorter)                        # 注册一个 Sorter 实例
runner.clear_sorters()                           # 清空已注册
results = runner.run(raw_data, config)           # list[int], BenchmarkConfig → list[BenchmarkResult]
dataset_result = runner.run_dataset(dataset, config)   # Dataset, BenchmarkConfig → BenchmarkDatasetResult
suite_result = runner.run_suite(suite, config)         # DatasetSuite, BenchmarkConfig → BenchmarkSuiteResult
```

### BenchmarkConfig

```python
config = sb.BenchmarkConfig()
config.dataset_id   = "default_dataset"   # str
config.input_id     = "input_0"           # str
config.data_type    = "unknown"           # str
config.repeat_times = 1                   # int
config.timeout_ms   = 0                   # int, 0=禁用
```

---

## 3. BenchmarkResult 字段（B 读、C 读）

以下字段名即 Python 属性名（snake_case）：

```python
r.dataset_id          # str
r.input_id            # str
r.data_type           # str
r.data_size           # int
r.algorithm_name      # str
r.elapsed_ms          # float  均值
r.median_elapsed_ms   # float  中位数
r.stddev_elapsed_ms   # float  样本标准差
r.compare_count       # int    均值
r.move_count          # int    均值（含 swap 产生的 move）
r.swap_count          # int    均值
r.key_op_count        # int    compare_count + move_count
r.sorted_correctly    # bool
r.timed_out           # bool
r.error_message       # str
r.time_rank           # int    ResultRanker.rank() 后填充
r.operation_rank      # int
r.overall_score       # float
r.overall_rank        # int
```

B 的 UI 显示至少用到：`algorithm_name` `elapsed_ms` `key_op_count` `sorted_correctly` `time_rank` `overall_score`。

---

## 4. DataGenerator（B、C、D 共用）

### 单组数据

```python
sb.DataGenerator.random_data(size, min_val, max_val)                    # list[int]
sb.DataGenerator.sorted_data(size, start_val=0, step=1)                 # list[int]
sb.DataGenerator.reversed_data(size, start_val=-1)                      # list[int]
sb.DataGenerator.nearly_sorted_data(size, start_val=0, step=1, swap_ratio=0.05)
sb.DataGenerator.duplicate_data(size, min_val, max_val, distinct_count=5)
sb.DataGenerator.constant_data(size, value=42)                          # list[int]
```

### 按字符串或枚举生成

```python
sb.DataGenerator.generate("random", size, min_val=0, max_val=10000)     # list[int]
sb.DataGenerator.generate_by_type(sb.DataType.SORTED, size, ...)        # list[int]
```

支持的类型字符串：`"random"` `"sorted"` `"reversed"` `"nearly_sorted"` `"duplicates"` `"constant"`

### DataType 枚举

```python
sb.DataType.RANDOM
sb.DataType.SORTED
sb.DataType.REVERSED
sb.DataType.NEARLY_SORTED
sb.DataType.DUPLICATES
sb.DataType.CONSTANT
```

### 批量生成

```python
# 字符串方式
sb.DataGenerator.generate_dataset("random", input_count, input_size, min_val, max_val)
# → list[list[int]]

# DatasetSpec 方式
spec = sb.DatasetSpec()
spec.data_type  = sb.DataType.RANDOM
spec.input_size = 1000
spec.input_count = 5
spec.min_value  = 0
spec.max_value  = 10000
dataset = sb.DataGenerator.generate_dataset_from_spec(spec)  # → Dataset

# Suite: 笛卡尔积
grid = sb.DatasetGridSpec()
grid.data_types  = [sb.DataType.RANDOM, sb.DataType.SORTED]
grid.input_sizes = [100, 500, 1000]
grid.input_count_per_dataset = 5
suite = sb.DataGenerator.generate_suite(grid)               # → DatasetSuite

# Suite: 按比例混合
mix = sb.DatasetMixSpec()
mix.input_size = 1000
mix.total_input_count = 10
part = sb.DatasetMixPart()
part.data_type = sb.DataType.NEARLY_SORTED
part.ratio = 0.5
mix.parts = [part, ...]
suite = sb.DataGenerator.generate_suite_from_mix(mix)       # → DatasetSuite
```

---

## 5. Dataset / DatasetSuite 结构（B、C 用）

```python
# Dataset
dataset.spec       # DatasetSpec
dataset.inputs     # list[DatasetInput]
dataset.id()             # str
dataset.data_type_name() # str: "random" ...
dataset.input_count()    # int

# DatasetInput
inp.id      # str
inp.values  # list[int]

# DatasetSuite
suite.datasets        # list[Dataset]
suite.dataset_count()   # int
suite.input_row_count() # int
```

---

## 6. ResultRanker（B、D 用）

```python
sb.ResultRanker.rank(results)   # 原地修改，填充 rank 字段
```

排名规则：竞赛排名（1 2 2 4），`sorted_correctly=False` 的沉底。

---

## 7. ReportWriter（B 用）

```python
sb.ReportWriter.write_console(results)                    # 打印表格
sb.ReportWriter.write_suite_console(suite_result)         # 打印 suite 表格
sb.ReportWriter.write_csv(results, file_path)             # 导出 CSV
sb.ReportWriter.write_suite_csv(suite_result, file_path)  # 导出 suite CSV
```

---

## 8. TrainingExport（C 用）

CSV 列（`write_training_csv` 生成）：

```text
dataset_id, input_id, data_type, size, data,
best_algorithm, best_time_ms, best_key_ops, best_moves, best_swaps, best_correct,
all_algorithms, all_times_ms, all_key_ops, all_moves, all_swaps
```

其中多值列用分号分隔（如 `all_algorithms: "QuickSort;BubbleSort;..."`）。

```python
# 单行
sb.TrainingExport.write_training_csv(raw_data, config, ranked_results, file_path)

# 按 dataset
sb.TrainingExport.write_training_dataset_csv(inputs, config, ranked_results_by_input, file_path)
sb.TrainingExport.write_training_dataset_csv_from_result(dataset_result, file_path)

# 按 suite
sb.TrainingExport.write_training_suite_csv(suite_result, file_path)

# 序列化
sb.TrainingExport.serialize_data(raw_data)   # "42 17 99 ..."
```

---

## 9. ML 推荐接口（C → B）

C 提供以下文件和函数签名：

### `ml/recommend.py`

```python
def recommend(data: list[int]) -> dict:
    """
    返回格式:
    {
        "algorithm": str,      # 推荐的算法名，如 "QuickSort"
        "confidence": float,   # 0~1
    }
    """
```

### `ml/model.joblib`

C 训练产出，由 `recommend.py` 内部加载，外部无需关心。

### B 调用方式

```python
from ml.recommend import recommend

r = recommend(raw_data)
print(r["algorithm"])   # "QuickSort"
print(r["confidence"])  # 0.82
```

---

## 10. 目录结构

```
project/
├── sorting_bench/          # C++ 扩展，import sorting_bench（已有）
├── app.py                  # Streamlit UI（B）
├── ml/
│   ├── features.py         # 特征提取（C）
│   ├── train_selector.py   # 训练脚本（C）
│   ├── model.joblib        # 训练产出（C）
│   └── recommend.py        # 推荐接口（C）
├── experiments/
│   ├── demo_pipeline.py    # 端到端脚本（D）
│   ├── benchmark_results.csv    # 输出
│   └── training_data.csv        # 输出
├── docs/
│   └── report.md           # 报告（D）
└── README.md               # 运行说明（D）
```

---

## 11. 依赖（`requirements.txt`）

```text
streamlit
pandas
plotly
scikit-learn
joblib
```

注：`sorting_bench` 是 C++ 编译产物（`.pyd`），不在 pip 依赖中，需单独构建。
