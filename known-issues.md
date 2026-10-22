# easySTL 已知问题与验证记录

> 整理日期：2026-10-21。基于当时源码的完整静态读取及有限编译、运行和定向复现。
> 扫描时 HEAD：`a7d7d23f79f65291a19ed8e12e3a9249e23553af`。后续源码变化后，需要重新核对位置和结论。
> 2026-10-22：CMake 已移除。内存池的实体定义在 `pool/allocator.cpp`，由 `pool/Makefile` 生成 `libeasypool.so`。`test/` 是后来补上的整体测试，用 `-I ../pool/include` 找到 `allocator.h`。下文里的文件路径和行号仍是 2026-10-21 的记录，不能直接当作当前源码位置。
> 本文是问题入口，尚未实施修复。下列优先级是建议处理顺序，不要求一次完成全部项目。

## 从哪里开始

先看 **E01：批量插入时扩容不足**。它有短复现，涉及 size、capacity、元素迁移和内存边界，适合先找回代码的调用关系。

先确认错误如何发生，再修改与验证。修复时更新对应条目的状态、实际日期和验证结果；只有编译通过不能标记为已修复。

状态含义：

- **已复现／待修复**：已有运行、诊断器或链接错误支持。
- **静态发现／待验证**：源码能看出风险或边界，但尚未完成对应动态复现。
- **范围待明确**：可以选择暂不支持，但要让实现、文档和测试保持一致。

## 问题总览

| 编号 | 优先级 | 问题 | 当前证据 |
|---|---|---|---|
| E01 | 高 | 批量插入扩容没有覆盖实际所需大小 | 数量插入已由 ASan 复现；范围插入有同样计算，尚未单独运行 |
| E02 | 高 | 单元素 erase 析构了 end，而非最后一个有效元素 | 生命周期记录器已复现 |
| E03 | 高 | 拷贝赋值对未构造空间做赋值 | 非平凡类型已复现 |
| E04 | 高 | 范围 insert 缺少返回值 | 编译器警告、UBSan 已复现 |
| E05 | 中 | 扩容后的 insert 返回旧位置；区间 erase 返回值也需核对 | insert 已复现；erase 为静态发现 |
| E06 | 中 | allocator 头文件在多个编译单元中产生重复定义 | 2026-10-22 两份源文件链接通过，见该条 |
| E07 | 中 | noexcept、abort 与异常恢复策略没有区分 | 静态发现，支持范围待明确 |
| E08 | 中 | 泛型接口实际支持的类型和迭代器较有限 | 静态发现，支持范围待明确 |
| E09 | 中 | 内存池的重分配、对齐和线程安全边界不完整 | 静态发现，待专项验证 |
| E10 | 中 | 测试主要打印结果，未形成可靠的自动判定 | 已读测试；CTest 枚举为 0 项 |
| E11 | 低 | 教程与源码接口不同步，部分解释不严谨 | 文档和源码对照发现 |

## E01：批量插入时扩容不足

状态：**已复现／待修复**。

位置：[vector.h](easySTL/vector.h) 的两个 `InsertAux`，扫描时在 239、254 行；新容量计算在 240、256 行。

当前计算：

```cpp
const size_type newsize = Max(size() * 2, static_cast<size_type>(16));
```

只考虑旧大小的两倍，没有考虑本次插入的数量。例如已有 40 个元素，再插入 100 个元素，需要至少 140 个位置，但实际只分配 80 个位置。

最小复现：将以下文件临时保存为 `/tmp/easystl-bulk.cpp`，在仓库根目录编译。额外包含标准头是为了让本复现不受头文件自包含问题干扰。

```cpp
#include <type_traits>
#include <initializer_list>
#include "vector.h"

int main() {
    easystl::vector<int> v(40, 1);
    v.insert(v.begin(), 100, 7);
}
```

```bash
g++ -std=c++17 -g -O1 -fsanitize=address,undefined \
  -fno-omit-frame-pointer -I easySTL \
  /tmp/easystl-bulk.cpp -o /tmp/easystl-bulk
ASAN_OPTIONS=detect_leaks=0 /tmp/easystl-bulk
```

实际结果：默认分配器下 ASan 报 `heap-buffer-overflow`，写入链路经过 `uninitialized_fill_n`。这里关闭泄漏检测，只为聚焦这次越界，不表示已验证没有泄漏。

修复后应检查：

- `size()` 为 140，`capacity()` 至少为 140；不要求容量恰好等于 140。
- 插入后前 100 个元素为 7，后 40 个仍为 1。
- 插在中间和末尾时，原有元素顺序仍正确。
- 范围插入也覆盖相同的容量边界，不能只改数量插入。
- 空容器、零个插入和大数量的算术溢出分别考虑。
- ASan／UBSan 不再报告相关错误。

## E02：单元素 erase 析构位置错误

状态：**已复现／待修复**。

位置：[vector.h](easySTL/vector.h)，扫描时 89—93 行。

```cpp
Copy(pos + 1, end_, pos);
Destroy(end_);
--end_;
```

`end_` 指向最后一个元素之后的位置，并不是一个已构造的有效元素。当前顺序会析构错误位置，真正需要清理的尾部对象反而没有按预期结束生命周期。

实际验证：使用记录构造／析构地址的非平凡类型，构造 3 个元素后删除首元素，检测到 **1 次无效析构**；容器销毁后仍留下 **1 个未正常析构的已构造地址**。这些计数描述对象生命周期，不是将记录器中的剩余地址直接当作堆内存泄漏统计。

修复后应检查：删除开头、中间、末尾；元素序列和 size；返回位置；拥有资源的元素是否恰好析构一次。只用 int 无法验证析构是否正确。

## E03：拷贝赋值写入未构造空间

状态：**已复现／待修复**。

位置：[vector.h](easySTL/vector.h)，扫描时 39—55 行，重点为最后一个分支。

触发条件：当前容器 size 小于右侧 size，但当前容量足够。

当前代码先把右侧整个区间 `Copy` 到 `begin_`，再对新增尾部 `uninitialized_copy`。新增部分原先只是原始存储，不应先调用对象的赋值操作。

实际验证：左侧 size 为 1、右侧 size 为 3，使用能记录“对未构造地址赋值”的非平凡类型，记录到 **2 次非法生命周期内的赋值操作**。

修复后应区分旧元素的赋值与新增元素的构造，并覆盖容量不足、容量足够但 size 不足、缩短、自赋值等路径。对带资源类型和构造计数进行验证。

## E04：范围 insert 缺少返回值

状态：**已复现／待修复**。

位置：[vector.h](easySTL/vector.h)，扫描时 152—177 行。

空范围有 `return pos`，非空范围的路径最终没有 return，但函数返回类型是 iterator。

实际结果：

- 编译器报告 `control reaches end of non-void function`。
- 对一个含 3 个整数的容器插入两个整数，UBSan 报 `execution reached the end of a value-returning function without returning a value`。
- 原仓库演示在本环境以 SIGILL 退出；行缓冲输出最后停在范围 insert 调用提示处。独立 UBSan 复现提供了更明确的错误证据。

修复后应为所有路径返回符合约定的有效迭代器，同时处理扩容后旧位置失效的问题，不能只随意补一个 return。

## E05：insert／erase 返回值不可靠

状态：**部分已复现／待修复**。

位置：[vector.h](easySTL/vector.h) 的数量 insert（扫描时 123—146 行）和区间 erase（95—101 行）。

数量 insert 在重新分配存储后仍返回旧 `pos`。实测构造 16 个整数、在 begin 插入一个元素后，返回值不等于新的 begin。返回值指向旧存储，不能继续解引用使用。

区间 erase 返回原 `last`，与通常的“删除区间之后的位置”不一致；此项仅静态核对，尚未单独动态复现。

修复前先明确本项目是否沿用 std::vector 的返回契约。验证插入／删除开头、中间、结尾，空范围，以及重新分配存储的情况。

## E06：多编译单元重复定义

状态：**已修复**。2026-10-22。

位置：实体定义已移到 [pool/allocator.cpp](pool/allocator.cpp)。`MallocInOom`、`ReallocInOom`、`CustomerOomHandler`、`freelist_`、`freespacestart_`、`freespaceend_`、`mallocoffset_`、`ChunkAlloc`、`ReFill` 只在这个文件里定义，并编进 `libeasypool.so`。

验证：两个源文件各自包含 `vector.h` 后分别编译，再和 `libeasypool.so` 链接。编译前额外包含了 `<type_traits>` 和 `<initializer_list>`，因为 `vector.h` 自己还没有包含这两份标准头（见 E08）。链接没有出现 `multiple definition`，程序退出码为 0。这次只构造了很小的 `vector<int>`，没有覆盖 E01 的扩容错误。

## E07：异常策略需要明确

状态：**静态发现／待验证；范围待明确**。

位置：[vector.h](easySTL/vector.h)、[algo.h](easySTL/algo.h)、[uninitialized.h](easySTL/uninitialized.h)、[allocator.h](pool/include/allocator.h)。

多个执行分配、拷贝构造或赋值的接口声明为 noexcept；一旦元素操作抛异常，将终止程序。未初始化构造的异常分支会清理已构造对象后直接 abort，而不是让调用方处理错误。当前不能宣称支持一般可恢复异常安全。

可以明确只支持不抛异常的元素／失败即终止，也可以实现可恢复策略；两者应分别写清楚。本次没有执行抛异常元素和分配失败的故障注入。

## E08：泛型和头文件边界需要收紧

状态：**静态发现／待验证；范围待明确**。

位置：[iterator.h](easySTL/iterator.h)、[vector.h](easySTL/vector.h)。

- IteratorTraits 识别自定义的大写成员名，与标准迭代器的常见命名不一致；不能由接口叫 Iterator 就宣称支持所有标准迭代器。
- RangeInit、assign、范围 insert 直接使用 `last - first`，实际上依赖随机访问能力。
- 范围 insert 的部分路径默认构造 `T()` 再赋值，对非默认可构造类型有额外限制。
- 当前 vector 缺少 const 访问重载及移动／仅移动类型的完整支持，不应写成与 std::vector 等价。
- vector 头已直接包含 `<type_traits>` 和 `<initializer_list>`。2026-10-22 用只包含 `vector.h` 的源文件、并按 `-I pool/include -I easySTL` 编译通过。其余类型组合仍未逐一编译测试。

这些项目可以先声明支持范围，再逐步扩展，不要求实现完整标准库。上述类型组合和头文件自包含尚未逐一编译测试。

## E09：内存池的行为与使用边界

状态：**静态发现／待专项验证**。

位置：[allocator.h](pool/include/allocator.h)。

- Reallocate 在大小类别变化时释放旧空间再分配，未保留旧内容；若要提供类似 realloc 的语义，需要核对并修正契约。当前 vector 扩容未直接调用这一接口，不能把它当作 E01 的根因。
- 小块池按 8 字节分组，未形成对任意过度对齐类型的支持保证。
- 全局 free list、起止指针等没有同步保护，不应假设支持多线程并发分配。
- 零字节请求、首次空指针差值、尺寸乘加溢出等边界需要进一步检查，尚未逐项动态验证。
- 进程级池保留内存与真正失去可回收引用要分开判断；本次未作完整泄漏分析。

先明确是单线程学习分配器还是要支持更通用的使用，不凭“用了内存池”宣称更快。本次没有性能基准。

## E10：让测试能判断失败

状态：**现状已核对／待改进**。

位置：[test.cpp](test/test.cpp)、[vectortest.h](test/vectortest.h)、[test.h](test/test.h)、[测试构建配置](test/CMakeLists.txt)。

现有测试主要调用整数容器并打印结果，没有对全部预期结果自动判定；本次 CTest 枚举为 0 项，是没有注册 CTest，不等于没有任何测试代码。

优先给 E01—E06 中的已复现行为补充有明确失败判据的测试。至少覆盖非平凡类型、扩容边界、返回迭代器和多编译单元；失败应通过非零退出码被外部检查发现。不要只以“有输出、没有崩溃”作为通过。

## E11：教程与实现对齐

状态：**文档／源码对照发现，待处理**。

位置：[教程一](tutorial1.md)、[教程二](tutorial2.md)、[教程三](tutorial3.md)、[教程四](tutorial4.md)。

- 教程四展示 at、const_iterator 等接口，当前源码没有相应完整实现，需要区分演示设计与已落地功能。
- at 示例同时声明 noexcept 和 throw，异常语义不一致。
- “每次 new／malloc 都直接向操作系统申请内存”的表述过度简化。
- 内存池减少开销或显著提升性能的描述缺少当前项目的实测依据。

先解决代码行为，再更新对应讲解；不要求为了文档上的接口清单无条件增加功能。

## 本次构建与验证边界

- 环境：Ubuntu WSL2，GCC 13.3.0，CMake 3.28.3。
- 初始构建：Debug，额外开启 `-Wall -Wextra -Wpedantic`，编译成功但有警告。
- 原演示：本环境未完整运行通过，子进程以 SIGILL 退出。
- 独立复现：ASan／UBSan、对象生命周期记录器和多编译单元链接。
- 构建、临时测试在仓库副本或 `/tmp` 执行。当前 CMake 会把可执行文件写入源码目录的 bin，直接构建前应知晓这一行为。
- 没有穷尽输入、跨平台、异常注入、所有元素类型或性能测试。未验证条目已单独标注。
- 本次只新增问题文档；源码、教程和构建配置尚未修复。
