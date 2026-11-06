# easySTL

使用 C++20，从零实现以 vector 为核心的小型 STL、独立内存池 easypool 和自动化测试，通过编码与 review 学习现代 C++。

## 入口

- [PLAN.md](PLAN.md)：实现顺序、接口、执行逻辑和验收条件。
- [record.md](record.md)：实际进度、验证结果、学习疑问及下一步。
- [AGENTS.md](AGENTS.md)：与 AI 协作时的任务范围、review 方式和操作边界。

继续学习时先看 record.md，再阅读 PLAN.md 中对应的小节。可以自行编码，也可以让 AI 分步实现；每次完成一小部分并检查结果。

## 当前状态

当前已实现并测试 SimpleAllocator 的字节分配与归还，覆盖零请求、无效对齐和过度对齐。元素类型适配器尚未实现，根 Makefile 尚未创建。当前 review 状态、验证限制和下一步见 record.md。

计划采用 easystl/、easypool/ 两个独立模块，测试与短实验统一放在根 test/，只使用根目录一份 Makefile。目录和构建目标见 PLAN.md，按实现需要逐步建立；当前使用阶段一的直接编译命令，不能直接运行 make。

旧版源码、测试与教程保存在 Git 提交 `f18416a` 中。可用 `git show f18416a:known-issues.md` 查阅旧问题，实施对应功能时设置回归。
