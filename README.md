# easySTL

使用 C++20，从零实现以 vector 为核心的小型 STL、独立内存池 easypool 和自动化测试，通过编码与 review 学习现代 C++。

具体目标是自己实现一个 vector：支持追加、插入、删除和读取元素，在空间不足时申请更大的内存，并在使用结束后销毁元素、释放内存。先实现内存分配、指定位置的构造与析构、读取一段数据和给已有对象赋值的工具，再把它们组合成容器。完成 vector 后，独立实现内存池，最后检查 vector 改用池分配内存时是否仍然正确。

## 入口

- [PLAN.md](PLAN.md)：从自行实现 vector 的目标出发，先说明各模块要解决的问题和工具职责，再介绍具体名称、测试任务与接口；同时说明本项目的阶段安排及原因。
- [record.md](record.md)：实际进度、验证结果、学习疑问及下一步。
- [AGENTS.md](AGENTS.md)：与 AI 协作时的任务范围、review 方式和操作边界。
- [test/vector_reserve_demo.cpp](test/vector_reserve_demo.cpp)：观察标准库 vector 预留空间、创建元素和析构的短实验，编译运行命令见 PLAN.md 的开头示例。

第一次阅读先看 PLAN.md 开头的 STL 组成简介，了解每类组件在哪些操作中起作用、调用者通常直接使用什么、库在内部完成什么，以及本项目选择实现哪些部分。再看 vector 的例子与阶段安排，把各阶段对应到组件或支撑工作，分清“已经分配存储空间”和“已经在其中构造对象”。继续学习时先看 record.md，核对已有实现和当前疑问，再回到对应阶段的测试与实现逻辑。

阅读状态变化时，先看具体调用：reserve 预留空间，push_back 在需要时构造新元素，pop_back 销毁末元素，容器析构时再完成剩余清理。PLAN 会列明调用前的条件和调用后的结果，并区分标准库示例与本项目计划采用的实现策略。

可以自行编码，也可以让 AI 分步实现；每次完成一小部分并检查结果，也可以停留在当前例子上改进教程和 review。计划中的后续能力不代表已经实现，实际状态如下。

## 当前状态

当前已实现并测试 SimpleAllocator 的字节分配与归还，覆盖零请求、无效对齐和过度对齐。元素类型适配器尚未实现。根 Makefile 已提供按文件名选择单个程序的初版入口，准备在 WSL 中使用，尚未完成 Makefile 的运行验证。当前 review 状态、验证限制和下一步见 record.md。

当前仍围绕阶段一优化教程可读性，重点是理解为什么需要独立的分配工具，以及它与后续对象操作、vector 的关系；已有代码仍需继续阅读和 review。

计划采用 easystl/、easypool/ 两个独立模块，测试与短实验统一放在根 test/，只使用根目录一份 Makefile。目录和完整构建目标见 PLAN.md，按实现需要逐步建立。

在装有 g++ 和 GNU Make 的 WSL 环境中，进入仓库根目录后，初版 Makefile 的用法如下：

```bash
make                              # 编译默认的 vector_reserve_demo
make run                          # 编译并运行默认实验
make run NAME=allocator_test      # 编译并运行 test/allocator_test.cpp
```

新增单文件实验后，使用 `make run NAME=文件名（不含.cpp）`，程序输出到 build/，头文件依赖通过编译器生成的 .d 文件记录。当前只提供 all、run 目标；汇总全部测试、诊断器、内存池链接等入口尚未实现。原来的 Windows 手动编译命令仍可使用，见 PLAN.md 的实验说明。

旧版源码、测试与教程保存在 Git 提交 `f18416a` 中。可用 `git show f18416a:known-issues.md` 查阅旧问题，实施对应功能时设置回归。
