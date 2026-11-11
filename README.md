# easySTL

通过重写部分 STL，练习和巩固日常工作中较少用到的现代 C++ 语法，加深对 STL 内部实现的理解。实践采用 C++20，从动态数组容器 easystl::vector 开始，再实现独立内存池 easypool；目标是理解设计与语言机制，不追求替代标准库。

实现容器时先用 std::allocator<T>、std::construct_at 和 std::destroy_at。随着追加、复制、插入删除、范围与移动操作逐步完成，再实现独立内存池并接入容器；迭代器和算法工具按需学习。

## 入口

- [PLAN.md](PLAN.md)：实现路线、各阶段的测试任务、接口和内部逻辑。
- [record.md](record.md)：学习者自行维护的进度、验证结果、学习疑问及下一步。
- [AGENTS.md](AGENTS.md)：与 AI 协作时的任务范围、review 方式和操作边界。

第一次阅读从 PLAN.md 的组件简介、vector 使用示例和阶段安排开始，再按当前阶段的测试任务创建目录、编写测试和实现。存储空间与对象的短实验可按需要选做。

可以自行编码，也可以让 AI 分步实现。每次完成一小步并检查结果，将实际改动、验证和疑问写入 record.md；继续学习时先核对记录与实际文件。测试结果与理解情况分别记录。PLAN.md 保留实现目标，无需随个人进度改写。

## 代码与构建

练习中的库和测试由读者按步骤创建，参考答案目录 reference/ 待后续整理。easystl/ 与 easypool/ 保持独立，测试和短实验共用 test/Makefile，根目录汇总入口在阶段七建立。

当前工作区保留 easystl/include/allocator.h 中的 SimpleAllocator，以及标准工具实验 test/storage_lifetime_demo.cpp；尚未实现 easystl::vector 和 easypool。当前可用的实验命令是 `make -C test demo`，默认构建并运行 storage_lifetime_demo。其他教程接口和根目录测试命令属于后续目标，实际进度与验证结果见 record.md。

旧版源码、测试与教程保存在 Git 提交 `f18416a` 中。可用 `git show f18416a:known-issues.md` 查阅旧问题，实施对应功能时设置回归。
