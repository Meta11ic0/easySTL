# 项目协作说明

## 沟通与执行

- 使用简体中文，先给结论，表达自然、具体，保留代码原名；遵循用户在会话中指定的称呼和格式，不把称呼写入代码、文档或提交信息。
- 在已授权范围内直接完成修改和验证；仅要求分析或评审时不修改文件。
- 保留用户已有修改和未跟踪文件，不覆盖、删除或顺手重构无关内容。
- 交付时说明修改内容、实际验证结果与未验证项，不把推断当成已确认事实。

## 项目定位与入口

easySTL 是学习 STL 内存管理、迭代器、算法和容器实现的 C++ 项目，命名空间为 `easystl`。当前容器主要是 `vector`，不应假定具备完整标准库兼容性。

- `pool/include/allocator.h`：内存池的公开接口，只有 `MallocAllocator` 和 `MemoryPoolAllocator`。
- `pool/allocator.cpp`、`pool/Makefile`：内存池动态库 `libeasypool.so`。构建只用 `pool/include`，不依赖 `easySTL/`。
- `easySTL/allocator.h`：容器侧的 `Allo` 和 `AllocatorWrapper`。当前 `Allo` 为 `MemoryPoolAllocator`，不要根据 `USEMALLOC` 名称推断实际选择。它通过 `#include <allocator.h>` 使用内存池，编译时 `-I pool/include` 必须在 `-I easySTL` 前面。
- `easySTL/constructor.h`：对象构造与析构。
- `easySTL/iterator.h`：迭代器标签、traits 和辅助操作。
- `easySTL/uninitialized.h`：未初始化内存上的对象操作。它包含 `constructor.h`，不包含分配器头文件。
- `easySTL/algo.h`：基础算法。
- `easySTL/vector.h`：容器实现。它包含同目录的 `allocator.h`。编译使用容器的程序时要加 `-I pool/include -I easySTL`，并链接 `-leasypool`。
- `test/test.cpp`、`test/Makefile`：整体测试。`make -C test run` 会先构建内存池，再用 `-I ../easySTL -I ../pool/include` 编译并链接 `libeasypool.so`。
- `tutorial1.md` 至 `tutorial4.md`：学习笔记；与实现不一致时以源码为准，并指出差异。
- `known-issues.md`：既有问题及验证记录；修改相关模块前先阅读对应条目，结论需结合当前源码重新核对。

## 实现约定

- 遵循相邻代码的两空格缩进、头文件保护宏、命名空间和命名方式，不做全仓格式化。
- 代码使用 `std::enable_if_t` 等 C++14 特性，默认按 C++14 验证；引入更高版本要求前说明原因。
- 保持学习型实现的可读性，不直接用 `std::vector` 替换自实现容器；可在测试中使用标准容器作行为对照。
- 区分分配内存与构造对象，检查元素生命周期、size/capacity、扩容后的迭代器与返回值、异常路径和边界输入。
- 不以现有 `noexcept` 声明推断操作不会抛异常，不以头文件组织形式推断多编译单元链接安全。
- 修复缺陷时添加能自动判定结果的针对性验证；涉及生命周期时覆盖非平凡类型，涉及内存边界时考虑 ASan/UBSan。
- 仅在有验证证据时更新 `known-issues.md` 的对应状态、日期和结果；不顺带修复其他条目。

## 构建与验证

在项目根目录执行：

```bash
make
make test
make clean
```

`make` 只生成 `pool/libeasypool.so`。`make test` 会在内存池源码或头文件变化后重编再运行。测试程序带有指向 `pool/` 的 rpath，可以直接运行 `./test/stltest`。测试通过只说明这次用例成立，不能单独证明容器行为正确。修复缺陷时仍需能自动判定结果的验证；涉及生命周期时覆盖非平凡类型，涉及内存边界时考虑 ASan/UBSan。报告实际执行命令、警告和失败；已有缺陷导致验证失败时如实说明，不为通过检查而隐藏诊断。

## Git 边界

- 默认只进行 `status`、`diff`、`log`、`show`、`rev-parse`、`ls-tree`、`check-ignore` 等只读检查。
- `add`、`commit`（含 amend）、`switch/checkout`、`branch`、`cherry-pick`、`merge`、`rebase`、`reset`、`restore`、`clean`、`stash`、`fetch`、`pull`、`push`、`tag`、`worktree`、`submodule update`、`git config` 等写操作必须获得针对该操作的明确授权；间接执行同样适用。
- 修改代码不等于授权暂存、提交、切换分支或推送；授权不扩展到后续操作。
- 提交信息、标签说明、MR/PR 标题和描述仅包含用户要求的必要内容，不自动添加署名、生成来源或宣传链接；复制旧信息时同样遵守。
