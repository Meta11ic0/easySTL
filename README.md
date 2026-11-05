# easySTL

用于学习现代 C++ 的小型 STL 重写项目，目标是实现容器、可替换的分配后端以及自动验证测试。

旧版源码、构建、测试和文档保存在 Git 提交 `f18416a` 中。新实现从零按步骤推进，由 AI 分步实现，学习者阅读、review 并验证理解。

## 当前入口

- [学习计划](modern-cpp-learning-plan.md)：按分配后端、内存池、对象操作、迭代器、算法和 vector 组织的八个阶段，包含前置知识、伪代码和验证要求。
- [协作说明](AGENTS.md)：教学方式与实现约定。
- `learning/`：生命周期与原始存储实验，通过 Makefile 编译运行。
- 旧问题记录：用 `git show f18416a:known-issues.md` 查阅，供新设计逐项对照。

## 构建与运行

需要 GNU make 和支持 C++14 的编译器。在根目录运行：

```bash
make -C learning
make -C learning run
```

两个实验分别展示自动生命周期和原始存储上的显式构造/析构，失败时返回非零。它们共用 `learning/tracked.h`，不依赖内存池。

根目录尚无统一 Makefile，正式容器、分配后端和回归测试按计划逐步实现。
