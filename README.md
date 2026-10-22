# easySTL

用于学习 STL 实现的 C++ 项目，包含内存分配器、对象构造与析构、迭代器、基础算法和 `easystl::vector`。当前实现仍有已知缺陷，不代表完整的标准库兼容实现。

## 目录

| 路径 | 内容 |
| --- | --- |
| `easySTL/` | 容器、算法、迭代器，以及给容器用的 `AllocatorWrapper` |
| `pool/` | 内存池。公开头文件是 `include/allocator.h`，实现是 `allocator.cpp`，Makefile 生成 `libeasypool.so` |
| `test/` | 整体测试。编译时加上内存池的头文件路径，并链接 `libeasypool.so` |
| `tutorial1.md` 至 `tutorial4.md` | STL、分配器、迭代器和 vector 学习笔记 |
| [known-issues.md](known-issues.md) | 已知问题、复现方式与验证记录 |
| [AGENTS.md](AGENTS.md) | AI 协作的项目背景、实现约定和验证要求 |

## 构建

需要 GNU make 和支持 C++14 的编译器。当前没有第三方库依赖。在项目根目录执行：

```bash
make          # 只编 libeasypool.so
make test     # 编内存池并运行整体测试
make clean
make install PREFIX=/usr/local
```

`pool/` 可以单独构建，不依赖 `easySTL/`。`vector.h` 包含同目录的 `allocator.h`，后者再用 `#include <allocator.h>` 找到内存池。因此 `-I pool/include` 必须写在 `-I easySTL` 前面，否则会包含到容器侧的包装头。

```bash
g++ -std=c++14 -I pool/include -I easySTL app.cpp -L pool -leasypool
```

安装后内存池头文件在 `PREFIX/include/easypool/allocator.h`，容器头文件在 `PREFIX/include/easystl/`，动态库在 `PREFIX/lib/`。使用时把上面的两个 `-I` 换成这两个安装目录，`-leasypool` 不变。测试通过只说明当前用例成立，不表示容器行为全部正确。已知缺陷见 [known-issues.md](known-issues.md)。

## 参与修改

修改前阅读 [AGENTS.md](AGENTS.md) 及相关已知问题。修复容器行为时用可自动判定结果的用例验证；涉及对象生命周期时不能只测试 `int`。教程可能与源码不同步，具体接口以当前实现为准。
