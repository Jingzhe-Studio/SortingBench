# 分工

## 总目标

```
import sorting_bench
→ Streamlit 选参数 → 运行 6 个排序算法
→ 展示耗时和 key_ops 图表 → 导出 benchmark CSV
→ 训练智能选择器 → UI 展示推荐结果
```

## 目录约定

```
project/
├── sorting_bench/      # C++ 扩展（已有）
├── app.py              # Streamlit UI（B）
├── ml/
│   ├── features.py         # 特征工程（C）
│   ├── train_selector.py   # 模型训练（C）
│   ├── model.joblib        # 训练产出（C）
│   └── recommend.py        # 推荐接口（C）
├── experiments/
│   ├── demo_pipeline.py    # 端到端脚本（D）
│   ├── benchmark_results.csv
│   └── training_data.csv
├── docs/
│   └── report.md               # 报告（D）
└── README.md               # 运行说明（D）
```

---

## 成员 A：C++ 排序算法补齐与绑定

- 实现 BubbleSort、SelectionSort、InsertionSort、ShellSort、MergeSort（各 1 个 `.h` + 1 个 `.cpp`）
- 在 `bindings.cpp` 中注册上述 5 个算法，使 `sb.BubbleSort()` 等可从 Python 调用
- 编译通过，`import sorting_bench` 可调用全部 6 个排序器（+ 已有的 QuickSort）

验收：

```python
import sorting_bench as sb

sorters = [
    sb.BubbleSort(), sb.SelectionSort(), sb.InsertionSort(),
    sb.ShellSort(), sb.MergeSort(), sb.QuickSort(),
]
data = [5, 3, 1, 4, 2]
config = sb.BenchmarkConfig()
runner = sb.BenchmarkRunner()
for s in sorters:
    runner.add_sorter(s)
results = runner.run(data, config)
for r in results:
    assert r.sorted_correctly, f"{r.algorithm_name} failed"
print("All 6 passed")
```

覆盖要求：在 random、reversed、duplicates、constant 四种数据上各跑一次，全部 `sorted_correctly=True`。

---

## 成员 B：Streamlit UI

- `app.py`（单文件 Streamlit 应用）

功能：

1. sidebar 控制面板：算法多选（6 个 checkbox）、数据类型下拉、数据大小滑块、repeat 次数、timeout
2. 点击 Run 调用 `BenchmarkRunner.run()`，返回结果表格（`st.dataframe`）
3. 耗时柱状图（plotly，按算法分组）
4. key_ops 柱状图（plotly，按算法分组）
5. CSV 导出按钮（`st.download_button`）
6. 推荐结果展示区块（为 C 预留接口，调用 `ml/recommend.py`）

技术：Streamlit + pandas + plotly

验收：

```bash
streamlit run app.py
```

能完成：选择数据规模 → 选择数据类型 → 勾选算法 → 点击 Run → 看到表格 + 两张柱状图 → 下载 CSV。

---

## 成员 C：ML 智能选择器

- `ml/features.py` — 特征提取
- `ml/train_selector.py` — 生成训练数据 + 训练模型 + 评估（需包含至少一个 baseline 对比）
- `ml/recommend.py` — 推荐接口 `recommend(data: list[int]) -> dict`
- `ml/model.joblib` — 训练产出

验收：

```bash
python ml/train_selector.py
python -c "
from ml.recommend import recommend
r = recommend([5, 3, 1, 4, 2])
assert 'algorithm' in r
print(r)
"
```

---

## 成员 D：集成、报告与演示

D 是总控，贯穿全程同步接入。

1. 写 `experiments/demo_pipeline.py`：生成数据 → 注册 6 算法 → benchmark → rank → 导出 CSV → 导出训练数据 → 调用训练 → 推荐示例输入 → 打印推荐结果（组装配用，不重写 B/C 代码）
2. 写 `docs/report.md`：项目背景与目标、系统架构图、6 算法原理与复杂度（A 提供素材）、基准测试实验分析（B 截图 + C 数据）、ML 智能选择器实验分析、总结与展望
3. 收集 `screenshots/`（UI 截图 ≥ 3 张）
4. 写 `README.md`（从零安装到跑通全流程的说明）

验收：

```bash
pip install -r requirements.txt
python experiments/demo_pipeline.py
streamlit run app.py
```
