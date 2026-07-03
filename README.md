# SortingBench

C++ 排序算法基准测试框架，提供 Python 接口、Streamlit 可视化、ML 智能推荐。

## 快速开始

```bash
# 1. Python 3.13（conda 或官方安装包）
conda create -n sorting-bench python=3.13 -y
conda activate sorting-bench

# 2. 安装依赖
pip install -r requirements.txt

# 3. 克隆仓库
git clone https://github.com/Jingzhe-Studio/SortingBench.git
cd SortingBench

# 4. 编译 C++（A/D）
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --target sorting_bench --config Release
copy build\Release\sorting_bench*.pyd .   # 分发 .pyd 到项目根目录

# 5. 验证（B/C 拿到 .pyd 后）
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
