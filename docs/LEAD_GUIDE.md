# D（队长）上手须知

## 项目现状

- **C++ 框架**：`BenchmarkRunner`、`DataGenerator`、`ReportWriter`、`TrainingExport`、`ResultRanker` 全部完工，QuickSort 和 SlowSort 可用
- **Python 绑定**：`import sorting_bench`，全部 API 可调，文档在 `docs/API.md`
- **缺失的**：BubbleSort、SelectionSort、InsertionSort、ShellSort、MergeSort 还是空壳，A 负责补齐

## 0. 统一 Python 环境——最先做

团队四人必须用同一个 Python 版本（3.13），否则 `.pyd` 不兼容。

### 方案一（推荐）：conda 创建统一环境

```bash
# 没有 conda 的话，先装 Miniconda：
# https://docs.conda.io/en/latest/miniconda.html

conda create -n sorting-bench python=3.13 -y
conda activate sorting-bench
```

然后把 conda 环境名 `sorting-bench` 告诉团队所有人，大家都用这个环境。

### 方案二：官方安装包

从 https://www.python.org/downloads/release/python-3130/ 下载 `python-3.13.0-amd64.exe`，安装时勾选 "Add Python to PATH"。

### 验证版本一致

```bash
python --version
# 必须输出 Python 3.13.x
```

确认后安装依赖：

```bash
pip install -r requirements.txt
```

---

## 搭建编译环境（D 和 A 都要搞，B、C 不用）

仓库里**没有预编译的 .pyd**。A 负责写 C++ 代码，但 D 也需要能自己编译，否则 A 卡住了没法帮。

### 1. 装 Visual Studio 2022 Community（免费）

从 https://visualstudio.microsoft.com/zh-hans/downloads/ 下载，安装时勾选：

```text
☑ 使用 C++ 的桌面开发  （Desktop development with C++）
```

装完重启电脑。

### 2. 装 CMake

```bash
pip install cmake
cmake --version   # 验证
```

### 3. 找到 Python 路径

```bash
python -c "import sys; print(sys.executable)"
python -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"
python -c "import sysconfig; print(sysconfig.get_path('include'))"
```

输出三行，记下来。例如：

```text
C:\Users\xxxx\miniconda3\python.exe
C:\Users\xxxx\miniconda3\libs
C:\Users\xxxx\miniconda3\Include
```

LIBDIR 下的库文件名是 `python313.lib`（`13` 是 Python 3.13；`python312.lib` 对应 3.12）。

### 4. 打开正确的终端 — 最容易翻车

**必须用 "Developer PowerShell for VS 2022"**，不是普通 PowerShell 或 cmd。

在开始菜单搜 `Developer PowerShell`，蓝色图标。如果搜不到，打开 cmd 执行：

```bash
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
```

验证编译器：

```bash
cl 2>&1 | head -3
# 应看到 "Microsoft (R) C/C++ Optimizing Compiler"
```

### 5. 编译

在 Developer PowerShell 中：

```bash
cd SortingBench

# 把三行路径换成自己的
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release `
  -DPYTHON_EXECUTABLE="C:/Users/xxxx/miniconda3/python.exe" `
  -DPYTHON_LIBRARY="C:/Users/xxxx/miniconda3/libs/python313.lib" `
  -DPYTHON_INCLUDE_DIR="C:/Users/xxxx/miniconda3/Include"

cmake --build build --target sorting_bench --config Release
```

成功后 `build/Release/` 下生成 `sorting_bench.cp3xx-win_amd64.pyd`。

### 6. 验证

```bash
cd build/Release
python -c "import sorting_bench as sb; print(sb.QuickSort().name())"
# 输出: QuickSort
```

### 7. 把 .pyd 分给 B 和 C

B 和 C 不需要 VS。D 或 A 编译后把 `.pyd` 复制到项目根目录：

```bash
copy build\Release\sorting_bench*.pyd .
```

B、C 拿到后在项目根目录下就能 `import sorting_bench`。

> `.pyd` 在 `.gitignore` 里，不提交。每次 A 改 C++ 后重新编译分发。

### 常见编译问题

| 现象 | 原因 | 解决 |
|------|------|------|
| `'cmake' 不是内部或外部命令` | 没装 | `pip install cmake` |
| `'cl' 不是内部或外部命令` | 没在 Developer PowerShell | 搜"Developer PowerShell for VS 2022"打开 |
| CMake 捡到 Windows Store 的 Python 3.12 | 没指定路径 | 加上三个 `-DPYTHON_*` 参数 |
| `ModuleNotFoundError: No module named 'sorting_bench'` | .pyd 不在当前目录，或 Python 版本不匹配 | 确认 `.pyd` 后缀中的 `cp313` 和 `python --version` 一致 |
| FetchContent clone 失败 | 网络问题 | 挂梯子重试 |

---

## D 的职责

### 1. 确保第一小时内每人跑通最小闭环

- A：`python -c "import sorting_bench as sb; print(sb.QuickSort().name())"` 能执行
- B：`streamlit hello` 能出页面
- C：`python -c "from sklearn.ensemble import RandomForestClassifier; print('ok')"` 不报错
- D：`python -c "import sorting_bench as sb; print(sb.BenchmarkRunner())"` 不报错

谁卡住了立刻帮排查，不闷头半小时。

### 2. 每 30 分钟进度同步

群里发：

```text
进度报一下：
A: 几个算法可用了？
B: UI 哪些控件好了？
C: 特征提取跑通了吗？
D: demo_pipeline.py 到哪一步了？
```

每人一行，不用长。目的不是盯人，是让谁也不掉队。

### 3. D 的代码量最小，但看全局

- `experiments/demo_pipeline.py` — 组装 A/B/C 的产出，预计 30 行
- `docs/report.md` — 用 `interface_contract.md` 已有描述 + 各自的截图和数据填充
- 核心价值不在写代码，在保证四人四小时后交出完整东西

---

## 开工步骤

```bash
# 1. 克隆
git clone https://github.com/Jingzhe-Studio/SortingBench.git
cd SortingBench

# 2. 统一 Python 3.13 + 装依赖（按上面第 0 步做）

# 3. 编译 .pyd（按上面"搭建编译环境"走完）
#    编译完后把 .pyd 复制到项目根目录，方便 import

# 4. 验证
python -c "import sorting_bench as sb; print('ok')"
```

---

## 参考文档

| 文档 | 给谁 |
|------|------|
| `docs/interface_contract.md` | 全员 — API 签名、字段名、目录结构 |
| `docs/TASK_DIVISION.md` | 全员 — 每人的交付物 |
| `docs/git_guide.md` | 全员 — 分支、提交、合并 |
| `docs/API.md` | B、C — Python API 详细参数 |
| `docs/BUILD.md` | A — C++ 编译细节 |

## 发群里的第一句话

```text
@所有人 4 步走：

1. 装 Python 3.13（conda create -n sorting-bench python=3.13 -y）
2. conda activate sorting-bench && pip install -r requirements.txt
3. git clone https://github.com/Jingzhe-Studio/SortingBench.git
4. python --version 确认是 3.13

跑不通马上说，我来帮看。30 分钟后第一次进度同步。

.pyd 我编译完发群里，其他人先不管 C++ 编译这一步。
```
