# SortingBench

C++ 排序算法基准测试框架，提供 Python 接口、Streamlit 可视化、ML 智能推荐。

## 快速开始

先说清楚分工：**写 Python 的**（B/C）不需要碰 C++；**写 C++ 的**（A/D）负责编译。下面按这个分。

### 所有人第一步：搞定 Python 3.13 + 克隆项目

先确认 Python 版本：

```bash
python --version
# Python 3.13.x → 完美，下一步
# 其他版本     → 需要装一个 3.13
```

如果版本不对，选一种方式装上 3.13：

| 方式 | 适合谁 | 怎么做 |
|------|--------|--------|
| **官方安装包** | 所有人，最简单 | [python.org](https://www.python.org/downloads/) 下载 3.13，一路下一步 |
| **conda** | 电脑上已经有 Anaconda/Miniconda 的同学 | `conda create -n sorting-bench python=3.13 -y`，以后每次打开终端先 `conda activate sorting-bench` |

> **conda 是什么？** conda 是 Python 的"环境管家"——你可以同时装 3.12 和 3.13 两个版本，互不打架，一键切换。没听说过就说明不需要，走官方安装包即可。

装好 Python 3.13 后，克隆项目并安装依赖：

```bash
git clone https://github.com/Jingzhe-Studio/SortingBench.git
cd SortingBench
pip install -r requirements.txt
```

### B / C：写 Python 的同学

你不需要编译 C++。让 A 或 D 编译好 `.pyd` 文件放到项目根目录，然后验证：

```bash
python -c "import sorting_bench as sb; print(sb.QuickSort().name())"
# 输出: QuickSort
```

之后正常开发即可。参考 [API.md](docs/API.md) 了解所有可用接口。

### A / D：写 C++ / 负责编译的同学

需要提前装好：

- **Visual Studio 2022**（"使用 C++ 的桌面开发" 工作负荷）
- **CMake** ≥ 3.18（`pip install cmake`）
- **Python 3.13**

然后编译 Python 模块：

```bash
# 在 Developer PowerShell for VS 2022 中
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --target sorting_bench --config Release
```

编译完后把 `.pyd` 复制到项目根目录，分发给 B/C：

```bash
copy build\Release\sorting_bench*.pyd .
```

详细说明见 [BUILD.md](docs/BUILD.md)。

### 验证一切就绪

```bash
python -c "import sorting_bench as sb; print('ok')"
```

## 文档索引

| 你是 | 先读 | 再读 | 参考 |
|------|------|------|------|
| **D（队长）** | [LEAD_GUIDE.md](docs/LEAD_GUIDE.md) | [TASK_DIVISION.md](docs/TASK_DIVISION.md) | [interface_contract.md](docs/interface_contract.md) |
| **A（算法）** | [BUILD.md](docs/BUILD.md) | [API.md](docs/API.md) | [interface_contract.md](docs/interface_contract.md) |
| **B（UI）** | [API.md](docs/API.md) | [TASK_DIVISION.md](docs/TASK_DIVISION.md) | [interface_contract.md](docs/interface_contract.md) |
| **C（ML）** | [API.md](docs/API.md) | [TASK_DIVISION.md](docs/TASK_DIVISION.md) | [interface_contract.md](docs/interface_contract.md) |

| 文档 | 内容 |
|------|------|
| [LEAD_GUIDE.md](docs/LEAD_GUIDE.md) | D 上手：环境搭建（VS2022+CMake+Python3.13）、职责、开工步骤 |
| [TASK_DIVISION.md](docs/TASK_DIVISION.md) | 四人分工：每人交付物与验收标准 |
| [interface_contract.md](docs/interface_contract.md) | 跨模块接口：算法类名、API 签名、CSV 字段、目录结构 |
| [git_guide.md](docs/git_guide.md) | Git 规范：分支策略、文件归属、提交格式、合并顺序 |
| [BUILD.md](docs/BUILD.md) | C++ 编译：CMake 配置、MSVC 命令、Python 路径指定 |
| [API.md](docs/API.md) | Python API 参考：全部类型、方法签名、用法示例 |
| [CLAUDE.md](docs/CLAUDE.md) | 项目架构说明（供 Claude Code 使用） |

## 目录

```
SortingBench/
├── sorting-bench/            # C++ 源码 + pybind11 绑定
│   ├── src/algorithms/       # 排序算法实现
│   ├── src/benchmark/        # BenchmarkRunner、结果结构体
│   ├── src/data/             # 数据生成、Dataset、I/O
│   ├── src/postprocess/      # 排名、报告、训练导出
│   └── src/pybind/           # pybind11 绑定代码
├── CMakeLists.txt            # CMake 构建（三目标）
├── app.py                    # Streamlit UI（B 负责）
├── ml/                       # ML 模块（C 负责）
├── experiments/              # 端到端脚本与输出（D 负责）
├── docs/                     # 全部文档
└── requirements.txt          # Python 依赖
```
