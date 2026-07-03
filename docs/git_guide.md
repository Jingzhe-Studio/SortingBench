# Git 协作规范

## 仓库

```
https://github.com/Jingzhe-Studio/SortingBench
```

---

## 分支策略

每个人在自己的分支上开发，完成后合并到 `main`。

```text
main              ← 公共基线，只合并不直接提交
├── feat/algorithms    ← 成员 A
├── feat/ui            ← 成员 B
├── feat/ml            ← 成员 C
└── feat/integration   ← 成员 D
```

**规则**：

- `main` 始终保持可运行状态
- 每个人的提交只在自己的分支上，不交叉
- 合并到 `main` 前先在本地验证：`python experiments/demo_pipeline.py` 通过

---

## 各成员负责的文件

| 成员 | 分支 | 负责的文件和目录 |
|------|------|------------------|
| A | `feat/algorithms` | `sorting-bench/src/algorithms/BubbleSort.{h,cpp}` `SelectionSort.{h,cpp}` `InsertionSort.{h,cpp}` `ShellSort.{h,cpp}` `MergeSort.{h,cpp}`、`sorting-bench/src/pybind/bindings.cpp`、`CMakeLists.txt` |
| B | `feat/ui` | `app.py` |
| C | `feat/ml` | `ml/` 目录下全部文件 |
| D | `feat/integration` | `experiments/` `docs/` `README.md` `requirements.txt` |

**关键**：每个人只改自己负责的文件，不碰别人的。不同分支改不同的文件，不会有 merge conflict。

`interface_contract.md` 由 D 维护在 `feat/integration` 分支上；A 如果需要改 `bindings.cpp` 的算法类名，也应照此文档来。

---

## 提交信息格式

```text
<type>: <简短描述>
```

| type | 场景 |
|------|------|
| `feat` | 新功能 |
| `fix` | 修 bug |
| `docs` | 文档 |
| `chore` | 构建、依赖、配置 |

示例：

```text
feat: implement BubbleSort
fix: SelectionSort incorrect on duplicate data
docs: add build instructions to API.md
chore: add matplotlib to requirements.txt
```

---

## 操作流程

### 第一步：克隆仓库

```bash
git clone https://github.com/Jingzhe-Studio/SortingBench.git
cd SortingBench
```

### 第二步：创建自己的分支

```bash
# 以 A 为例
git checkout main
git pull origin main
git checkout -b feat/algorithms
```

### 第三步：日常开发

```bash
# 修改代码...
git add <改的文件>
git commit -m "feat: implement BubbleSort"
git push origin feat/algorithms    # 首次推送
# 后续推送只需 git push
```

### 第四步：合并到 main

```bash
# 1. 确保本地 main 最新
git checkout main
git pull origin main

# 2. 合并自己的分支
git merge feat/algorithms

# 3. 推送到远程
git push origin main
```

### 合并顺序建议

按依赖关系依次合并：

```text
feat/algorithms → main    （最先，B 和 C 需要 A 的算法）
feat/ml → main            （C 需要 A 的算法跑训练数据）
feat/ui → main            （B 需要 A 的算法 + C 的 recommend 接口）
feat/integration → main   （D 最后，集成所有产出）
```

如果 D 提前写的 `experiments/demo_pipeline.py` 只需要已有 API，不用等也能先合。

---

## 冲突处理

每人改不同文件，正常情况下不会有冲突。唯一的冲突点是 `ExperimentalSort` 等空壳占位文件（多人删除/修改它们）。

如果发生冲突：

```bash
# merge 时报冲突
git status                  # 看哪些文件冲突

# 编辑冲突文件，保留正确的版本，删除 >>> <<< 标记

git add <冲突文件>
git commit -m "fix: resolve merge conflict"
git push origin main
```

---

## 常用命令速查

```bash
git status                    # 看改了哪些文件
git diff                      # 看具体改了什么
git log --oneline -10         # 看最近 10 条提交
git stash                     # 暂存当前修改，切分支
git stash pop                 # 恢复暂存的修改
git reset --hard HEAD~1       # 撤销最近 1 次提交（未推送时）
git checkout -- <file>        # 丢弃某个文件的修改
```

---

## 禁止事项

- 不要直接在 `main` 上写代码提交
- 不要把 `.pyd` `.exe` `.lib` `build/` `__pycache__/` 等产物加入 git（`.gitignore` 已配置）
- 不要 `git push --force`（强制推送会覆盖别人的提交）
- 不要在一个 commit 里混多个人的文件改动
