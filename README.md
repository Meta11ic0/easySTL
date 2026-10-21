# easySTL

用于学习 STL 实现的 C++ 项目，包含内存分配器、对象构造与析构、迭代器、基础算法和 `easystl::vector`。当前实现仍有已知缺陷，不代表完整的标准库兼容实现。

## 目录

| 路径 | 内容 |
| --- | --- |
| `easySTL/` | 核心实现头文件 |
| `test/` | 示例与测试入口 |
| `tutorial1.md` 至 `tutorial4.md` | STL、分配器、迭代器和 vector 学习笔记 |
| [known-issues.md](known-issues.md) | 已知问题、复现方式与验证记录 |
| [AGENTS.md](AGENTS.md) | AI 协作的项目背景、实现约定和验证要求 |

## 构建与运行

需要 CMake 和支持 C++14 的编译器。当前没有第三方库依赖。在项目根目录执行：

```bash
cmake -S . -B build -DCMAKE_CXX_STANDARD=14 -DCMAKE_CXX_STANDARD_REQUIRED=ON
cmake --build build
./bin/stltest
```

当前 CMake 配置固定把可执行文件输出到项目根目录的 `bin/`；`build/` 和 `bin/` 已加入忽略规则。多配置生成器可能在 `bin/` 下生成对应配置子目录，运行时需调整路径。

测试入口调用 `VectorTest()`，主要通过打印容器内容演示行为，尚未注册 CTest 测试，也没有完整的自动断言。运行完成不等于所有容器行为正确。

2026-10-21 初始化验证：在 GNU C++ 13.3.0、C++14 配置下构建成功，编译器报告范围 `insert` 缺少返回值；执行 `./bin/stltest` 的退出码为 132，未正常完成。本次未确认该运行失败的具体原因。既有缺陷的独立记录见 [known-issues.md](known-issues.md)。

## 参与修改

修改前阅读 [AGENTS.md](AGENTS.md) 及相关已知问题。修复容器行为时用可自动判定结果的用例验证；涉及对象生命周期时不能只测试 `int`。教程可能与源码不同步，具体接口以当前实现为准。
