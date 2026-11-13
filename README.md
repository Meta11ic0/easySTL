# easySTL

通过重写部分 STL，练习和巩固日常工作中较少用到的现代 C++ 语法，加深对 STL 内部实现的理解。实践采用 C++20，从动态数组容器 easystl::vector 开始，再实现独立内存池 easypool；目标是理解设计与语言机制，不追求替代标准库。

实现容器时先用 `std::allocator<T>`、std::construct_at 和 std::destroy_at。先完成容器的存储、复制和位置修改，再实现自制迭代器与 iterator_traits、范围和移动操作。随后实现普通分配器、独立内存池，以及简化 allocator_traits 和池接入，分八阶段推进。

## 入口

- [PLAN.md](PLAN.md)：实现路线、各阶段的测试任务、接口和内部逻辑。
- [RECORD.md](RECORD.md)：实际进度、验证结果、学习疑问及下一步。
- [AGENTS.md](AGENTS.md)：与 AI 协作时的任务范围、review 方式和操作边界。

第一次阅读从 PLAN.md 的组件简介、vector 使用示例和阶段安排开始，再按当前阶段的测试任务创建目录、编写测试和实现。存储空间与对象的短实验可按需要选做。

可以自行编码，也可以让 AI 分步实现。每次完成一小步就检查结果；AI 协作时由 AI 更新 RECORD.md，继续学习时核对记录与实际文件。测试结果与理解情况分别记录。PLAN.md 保留实现目标，无需随个人进度改写。

## 代码与构建

练习中的库和测试由读者按步骤创建，参考答案目录 reference/ 待后续整理。easystl/ 与 easypool/ 保持独立，测试和短实验共用 test/Makefile，根目录汇总入口在阶段八建立。

当前已重新建立标准工具实验 test/storage_lifetime.cpp 和共用 test/Makefile。执行 `make -C test run`，或显式选择 `make -C test NAME=storage_lifetime run`，编译并运行空间分配与对象构造的三个对照过程；只构建时使用 `make -C test`。尚未重新实现 easystl 或 easypool，实验结果不计为容器功能验收。实际进度与历史验证见 RECORD.md。

旧版源码、测试与教程保存在 Git 提交 `f18416a` 中。可用 `git show f18416a:known-issues.md` 查阅旧问题，实施对应功能时设置回归。
