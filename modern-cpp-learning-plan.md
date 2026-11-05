# 从零实现小型 STL：模块实现与现代 C++ 实践计划

## 一、项目目标

以构建一套可以编译、使用和测试的小型 STL 为主线，实现基础 vector、内存池、对象操作、迭代器工具和必要算法。实现范围以项目备份版本为基础，在重写中修正缺陷并逐步增加移动、转发和异常安全支持，不扩展为完整 C++ 标准库。

阶段按实际模块与功能划分。生命周期、RAII、模板推导和 SFINAE 等知识附属于对应实现：遇到不理解的代码，再围绕当前问题进行短实验和解释，不先展开成独立的长期基础课程。

代码可以自行编写，也可以由 AI 分步实现。无论采用哪种方式，都应先确定接口和行为，再阅读实现、运行验证、审查边界与失败路径。每个阶段表示一个模块的建设范围，不表示一次生成整个模块。

## 二、实现范围

主体包含：

- 简单原始存储分配后端和小块内存池后端。
- 按元素类型适配的分配接口，以及构造、析构和未初始化操作。
- 迭代器类别、traits、距离与位置移动工具。
- vector 所需的基础算法。
- vector 的构造、访问、容量管理、插入、删除、赋值与交换。
- 移动语义、仅移动元素、emplace_back 与适用的异常保证。
- 自动化测试、独立模块构建和整体集成。

暂不实现 list、deque、map、unordered_map 或完整 allocator 传播协议。多线程池、复杂性能优化和新标准对照属于扩展，不阻塞主体完成。

## 三、结构与实施方式

```text
include/easystl/
  allocator.h              类型分配适配，连接分配后端
  constructor.h            对象构造与析构
  uninitialized.h          未初始化存储上的批量对象操作
  iterator.h               迭代器类别、traits 和辅助操作
  algorithm.h              基础算法
  vector.h                 容器
pool/
  include/easypool/
    allocator.h            简单分配后端与内存池接口
  allocator.cpp            需要独立编译的后端实现
  Makefile
learning/
  Makefile
  *.cpp                    按实际疑问编写的小实验
test/
  Makefile
  *.cpp                    各模块与整体回归测试
Makefile                   统一构建与测试入口
README.md                  使用方式、支持范围与限制
```

正式库使用 C++14、`easystl` 命名空间和两空格缩进。头文件采用明确前缀，例如 `<easystl/vector.h>` 和 `<easypool/allocator.h>`，避免同名文件依赖包含路径先后顺序。

目录和文件随模块需要建立。独立实验不强制命名空间、测试类名称、共享头文件或计数系统。重复少量实验代码可以接受；出现明确复用需求后再提取。

每次实施只处理当前讨论的一项接口或一条执行路径。先直接说明它在模块中的作用，再实现、解释和验证。遇到知识疑问时停留在当前代码，不为提前覆盖知识点添加无关功能，也不因模块清单较长而一次性生成全部实现。

### 模块依赖与完成顺序

```text
SimpleAllocator ───────────────┐
       ↑                      │
MemoryPool                    │
       └── 后端句柄 ── AllocatorWrapper<T, Backend>
                                  │
Construct / Destroy ── uninitialized_*   IteratorTraits
                 │                       │
                 └──────── vector ── 基础算法
```

分配后端管理字节，类型适配管理元素数量到字节数的转换，对象操作管理生命周期，vector 组合这些能力。阶段顺序采用基础设施到容器的方向，但迭代器与算法可直接用数组及标准迭代器测试，不必等待 vector。

内存池先独立完成，vector 开始时仍先使用简单后端，以免容器问题与池问题混淆。阶段七再完善移动与转发，阶段八统一验收。接口框架和伪代码描述阶段最终应覆盖的路径，实施时逐项展开，不表示一次生成完整模块。

### 分配资源的项目约定

统一后端接口接收字节数与对齐：`Allocate(bytes, alignment)`、`Deallocate(pointer, bytes, alignment)`。零尺寸返回空指针，空指针释放无操作；失败抛出异常。第一版普通后端只支持明确的普通对齐范围，不支持的对齐在分配前拒绝，不静默返回错误地址。

类型适配器保存可复制的后端句柄。简单后端句柄无状态，池句柄借用一个不可随意复制的池对象。复制句柄不复制池，也不延长池寿命，客户容器必须先于池销毁。仅复制句柄不抛异常，句柄连同存储一起交换也不抛异常；若实际设计不满足这些前提，必须重新审查相关保证。

为缩小实现范围，容器拷贝构造使用来源的资源，初版拷贝赋值采用临时副本与交换，因此目标可改用来源资源；移动和 swap 同时转移或交换句柄与存储。这些是本项目的规则，不宣称与标准 allocator 传播协议完全一致。若后续希望目标资源保持不变，另行实现不同资源间的元素迁移路径。

所有存储释放都由当时关联的资源执行。涉及替换存储时，应先保存旧指针、旧容量和旧资源，不在更新句柄后误用新资源释放旧块。

## 四、模块实现阶段

各阶段的接口草图和流程伪代码用于指导逐项实现，省略模板声明、包含文件及部分边界细节，不作为可直接编译的完整源码。实现时仍须满足对应的契约与验证要求。

### 阶段一：简单分配后端与类型适配

#### 目标

建立后续模块使用的存储接口。先有可以独立工作的简单分配后端，再替换或扩展为内存池。

#### 实现内容

- 实现基于 `::operator new/delete` 的简单后端，统一字节分配和释放，规定零请求、失败方式和对齐支持范围。
- 实现按元素类型适配的接口：接收元素数量，检查乘法溢出，转换为字节数并调用后端。分配接口不构造元素。
- 保持后端与 vector 独立，允许以后通过模板参数或明确的资源关联选择实现，不依靠宏名称隐式切换。
- 建立最小模块 Makefile 与直接调用测试，不在本阶段实现容器或通用测试框架。

#### 前置知识

需要时解释分配函数与 new 表达式的区别、`sizeof(T)`、基本对齐、模板类型适配和分配失败。知识解释服务于分配接口，不要求先写一套对象观察工具。

#### 接口框架

以下是接口草图，省略包含、命名空间和实现细节，不作为可直接编译的源码。

```cpp
class SimpleAllocator {
 public:
  void* Allocate(size_t bytes, size_t alignment);
  void Deallocate(void* p, size_t bytes, size_t alignment) noexcept;
};

template<class T, class Backend = SimpleAllocator>
class AllocatorWrapper {
 public:
  explicit AllocatorWrapper(Backend backend);
  T* Allocate(size_t n);
  void Deallocate(T* p, size_t n) noexcept;
 private:
  Backend backend_; // 后端句柄；不是池本体
};
```

#### 运行流程

```text
AllocatorWrapper<T>.Allocate(n):
    n == 0 → 返回空指针
    n > 最大可表示字节数 / sizeof(T) → 抛出尺寸异常
    bytes = n * sizeof(T)
    检查后端支持 alignof(T)
    返回 backend.Allocate(bytes, alignof(T)) 转换后的地址

SimpleAllocator.Allocate(bytes, alignment):
    检查零请求与对齐范围
    调用 ::operator new(bytes)，让分配失败向外传播

AllocatorWrapper<T>.Deallocate(p, n):
    p 为空 → 返回
    按原数量、类型对齐和原后端归还存储
```

适配层禁止接受与原分配不匹配的数量。此阶段只验证存储接口，后续 Construct 才负责在地址上创建对象。

#### 验证与 review

检查正常与零请求、过大数量、分配失败及申请释放配对。确认没有隐式构造客户对象；明确普通对齐与过度对齐的支持边界，不能未经验证宣称支持任意类型。

### 阶段二：小块内存池

#### 目标

提供第二种可独立使用的存储后端，恢复备份项目中内存池模块的功能，并使其资源责任与边界清楚。

#### 实现内容

- 从固定尺寸块开始，实现空闲链表的取出与归还，再实现耗尽后的底层存储补充。
- 从第一次取得大块存储起记录释放责任。明确池的生命周期，池销毁前客户必须结束对象生命周期并归还存储。
- 扩展为多个大小类别，实现大小取整、索引、大块回退和对齐策略。最小块必须容纳空闲节点并满足其对齐。
- 处理补充失败和元数据记录失败，避免记录元数据时递归向自身分配。
- 第一轮不保留无明确用途的 Reallocate。若以后加入，单独规定内容保留与失败时原块的有效性。

#### 前置知识

在对应实现处解释侵入式链表、指针对齐、存储复用、RAII 和失败清理。先完成一个大小类别，不在第一次实现中同时加入全部优化。

#### 状态与接口框架

```text
MemoryPool:
    free_lists[类别数]        各类空闲块链表
    cursor, limit            当前补充区间的起止位置
    chunks                   拥有的底层块及其释放信息
    upstream                 简单后端

    Allocate(bytes, alignment)
    Deallocate(p, bytes, alignment)
    RoundUp(bytes)
    GetFreelistIndex(rounded_bytes)
    ReFill(block_size)
    ChunkAlloc(block_size, 期望块数) → 地址与实际块数
```

第一步只建立一个类别，之后再引入类别数组与补充区间。粒度需能满足节点对齐，最小块至少容纳节点；最大池请求值和每次补充数量作为可调策略，不固定要求沿用 8 字节、128 字节和 20 块。

#### 分配、归还与类别计算

```text
RoundUp(bytes):
    确认粒度为选定的合法值，先检查加法上限
    将 bytes 向上取整到粒度的倍数

GetFreelistIndex(rounded):
    rounded / 粒度 - 1         // 零请求已提前处理

Allocate(bytes, alignment):
    bytes == 0 → 返回空指针
    超出池大小或对齐支持范围 → upstream.Allocate
    block_size = RoundUp(max(bytes, 最小节点大小))
    链表非空 → 弹出一个节点并返回存储
    链表为空 → ReFill(block_size)

Deallocate(p, bytes, alignment):
    p 为空 → 返回
    按与 Allocate 相同的规则判断回退路径
    在归还存储中建立空闲节点并压回原类别
```

#### ReFill 与 ChunkAlloc

```text
ReFill(block_size):
    (chunk, count) = ChunkAlloc(block_size, 期望块数)
    第一块作为本次分配结果
    其余块逐个建立节点并加入该类链表
    返回第一块

ChunkAlloc(block_size, count):
    cursor 为空 → 剩余字节为 0，不做空指针相减
    否则 remaining = limit - cursor
    如果足以提供 count 块
        截取需要的区间，推进 cursor
    否则如果至少能提供一块
        减少实际 count，截取完整块区间
    否则
        申请新底层块，先确保它有明确清理责任
        记录 chunks 失败 → 释放新块并抛异常，保持旧状态有效
        成功后将旧尾部按合法类别回收，或保留为所属块内的未利用字节
        设置新 cursor/limit，再截取区间
```

向上游申请失败时先传播异常，第一轮不加入跨类别“拆大块救急”等复杂优化。底层块列表始终保有释放责任；尾部暂不使用也不能失去其所属底层块的记录。

#### 验证与 review

独立检查块不重叠、归还复用、耗尽补充、各类别边界、零尺寸、大块、对齐和溢出。注入底层及元数据失败，检查已有链表仍有效、底层块无遗失。对 C++14 过度对齐使用明确方案或明确拒绝。池保留可复用内存与丢失释放责任分别判断。

### 阶段三：对象操作与未初始化算法

#### 目标

在分配模块提供的存储上创建和销毁对象，为容器提供可复用的生命周期操作。

#### 实现内容

- 在 constructor.h 中实现单对象 Construct/Destroy，再加入区间销毁。
- 在 uninitialized.h 中实现未初始化拷贝、填充和数量填充；逐个记录成功构造的位置，失败时销毁自身已构造部分后传播异常。
- 明确职责：对象操作层清理对象，取得存储的外层所有者释放存储。不能重复释放，也不能将错误直接转为 abort。
- 先实现当前需要的参数形式，完美转发在现代化阶段再加入，不提前引入整套泛型构造框架。

#### 前置知识

对象生命周期与原始存储、placement new、显式析构、构造与赋值的区别、异常展开。若某一点不清楚，用单个普通测试对象作短实验，不要求固定名称或共享头文件。

#### 接口框架

```text
Construct(T* p)                         默认/值初始化一个对象
Construct(T* p, const U& value)          用 value 构造对象
Destroy(T* p)                           销毁一个活对象
Destroy(first, last)                    销毁半开区间
uninitialized_copy(first, last, out)    返回构造区间末尾
uninitialized_fill(first, last, value)
uninitialized_fill_n(out, n, value)     返回构造区间末尾
```

第一版目标为类型适配器取得的连续存储，不承诺支持任意代理迭代器或任意原始内存模型。地址取得使用明确的对象位置，不调用目标位置上的赋值操作。

#### 批量构造与外层清理

```text
uninitialized_copy(first, last, out):
    current = out
    try:
        当 first != last:
            Construct(current, *first)
            前移 first 与 current
    catch:
        Destroy(out, current)
        重新抛出异常
    返回 current

外层创建 n 个对象:
    p = allocator.Allocate(n)
    try:
        finish = uninitialized_fill_n(p, n, value)
    catch:
        allocator.Deallocate(p, n)  // 内层已销毁它成功构造的部分
        重新抛出异常
    成功后由外层拥有 [p, finish) 与存储
```

fill 与 fill_n 采用相同的进度记录；默认构造数量接口逐个构造，不能通过先创建一个 T() 再复制的方式无意要求类型可拷贝。Destroy 不负责存储释放。

#### 验证与 review

验证非平凡对象构造与析构配对，分配不会构造对象，未构造位置不被赋值或析构。逐位置注入构造失败，检查只清理成功部分。计数或地址记录在需要发现具体错误时再加入，不成为所有实验的前置条件。

### 阶段四：迭代器基础设施

#### 目标

建立算法和容器之间的类型与遍历接口，对应备份教程中的迭代器模块。

#### 实现内容

- 定义或适配迭代器类别和关联类型，支持原始指针与 const 指针。
- 让 traits 与标准迭代器命名和类别兼容；若采用自定义体系，明确转换或适配，不能混用后假定自动兼容。
- 实现 Distance 与 Advance，根据类别选择遍历或随机访问路径。
- 为后续范围接口准备必要的检测能力，但在实际重载出现时再引入具体 SFINAE 约束。
- vector 第一版使用指针作为迭代器，不额外实现不必要的包装类。

#### 前置知识

关联类型、模板偏特化、traits、标签分发；输入迭代器与前向迭代器在重复读取能力上的差异。

#### 类型框架

```text
IteratorTraits<Iterator>:
    iterator_category
    value_type
    difference_type
    pointer
    reference

IteratorTraits<T*>:
    category = random_access
    value_type = T
    reference = T&

IteratorTraits<const T*>:
    category = random_access
    value_type = T
    reference = const T&
```

类别采用标准标签或明确适配到标准标签，关联类型采用标准小写命名。对不满足条件的类型保留可检测的失败形式，不能仅因模板具有同名成员就认定其全部操作合法。

#### 分派与范围约束

```text
Distance(first, last):
    category = IteratorTraits<Iterator>.iterator_category
    随机访问 → last - first
    输入/前向/双向 → 循环 ++first，累计距离

Advance(it, n):
    随机访问 → it += n
    双向 → 按 n 正负调用 ++ 或 --
    输入/前向 → 仅允许非负 n，循环 ++

范围重载是否启用:
    检查关联类别与需要的表达式
    不满足 → 替换时移除该候选
    满足 → 进入相应类别的范围实现
```

C++14 下可自行定义 `template<class...> using void_t = void`，不无说明使用 C++17 的 std::void_t。约束在 vector 范围重载落地时展开，不要求此阶段实现所有元素操作约束。

#### 验证与 review

覆盖指针、const 指针、标准随机访问、链表/前向及输入流迭代器。按类别规定合法的移动方向，不执行输入迭代器的负向前进；不在通用路径无条件使用减法，不假定单遍来源可以重读。

### 阶段五：基础算法

#### 目标

提供 vector 实现所需的算法，保持算法与容器、存储分配的职责分离。

#### 实现内容

- 实现前向拷贝、后向拷贝、填充、数量填充、比较或最大值、交换等实际需要的基础操作。
- 明确算法处理的是已有对象还是未初始化存储，避免与 uninitialized 系列混淆。
- 明确重叠区间要求、输出范围前提和异常行为，不能因模板函数短小就统一声明 noexcept。
- 先使用清晰的逐元素实现，移动算法、条件 noexcept 和性能快路径随后续需求加入。

#### 前置知识

迭代器要求、重叠搬移方向、元素赋值与构造、泛型算法的类型要求。

#### 接口与循环框架

```text
Copy(first, last, out):
    当 first != last:
        *out = *first
        同时前移 first 与 out
    返回 out

CopyBackward(first, last, out_end):
    当 first != last:
        *--out_end = *--last
    返回 out_end

Fill(first, last, value):
    逐个执行 *first = value

FillN(out, n, value):
    赋值 n 次并返回末尾

Swap(a, b):
    第一版使用拷贝临时对象与赋值
    现代化阶段再加入移动与条件 noexcept
```

Copy 只承诺满足其重叠前提的范围；右移通常使用 CopyBackward。不要先用 memcpy 替换逐元素操作，非平凡类型、重叠与对象生命周期要求需要另外论证。Max 等返回引用的工具还需检查临时实参的寿命，避免保存悬空引用。

#### 验证与 review

验证空区间、单元素、普通区间与合法重叠方向。使用非平凡对象区分构造和赋值；元素赋值抛异常时不被错误 noexcept 终止。只与标准算法定义良好的用法对照。

### 阶段六：基础 vector

#### 目标

组合前述模块，实现与备份版本规模相近的基础 vector，并修正已知的容量、生命周期、返回值与范围接口问题。

#### 实现内容

- 先建立空状态、三指针不变量、析构、size/capacity/empty、begin/end/data 和 const 访问。空状态单独处理，避免空指针相减。
- 使用简单后端先完成容器逻辑，再以相同契约运行池后端。容器调试不依赖池一定正确，始终保留两种后端的独立测试。
- 逐项加入预留容量、追加、弹出、清空和扩容；增长覆盖实际需求并检查溢出。
- 加入数量、范围及初始化列表构造、深拷贝、赋值和成员 swap。数量与范围重载的约束同步实现，不能先留下冲突再集中处理。
- 逐项实现 erase、resize、单元素/数量/范围 insert 和 assign，分别分析扩容和不扩容路径。
- 明确自插入、内部引用、迭代器失效、返回位置和每个操作的类型要求。单遍输入不能先计数再重读，可先借助临时存储简化实现。
- 采用局部 RAII 管理新存储与部分构造，按操作说明异常保证；内部引用参数不能在读取前失效。

#### 前置知识

类模板、资源所有权、RAII、深拷贝、const、重载选择、SFINAE、迭代器失效和异常保证。每个知识点在对应接口出现时解析，不在开始实现 vector 前集中讲完。

#### 容器框架与接口范围

```text
vector<T, Backend = SimpleAllocator>:
    allocator_                    类型适配器，保存后端句柄
    begin_, end_, capacity_        空状态均为空指针

    默认、数量、范围、initializer_list、拷贝构造
    拷贝赋值、initializer_list 赋值、析构
    begin/end/cbegin/cend 与 const 版本
    size/capacity/empty/data
    operator[]/front/back 与 const 版本
    reserve/push_back/pop_back/clear
    erase(pos), erase(first, last)
    resize(n), resize(n, value)
    insert(pos, value), insert(pos, n, value), insert(pos, first, last)
    assign(n, value), assign(first, last), assign(initializer_list)
    swap

    私有辅助：数量初始化、范围初始化、容量检查、释放、扩容提交
```

`at` 可在基本访问稳定后作为有界访问补充，越界抛 out_of_range，不能声明为 noexcept。shrink_to_fit 不列为第一轮必需接口。模板默认后端不得以自身参数递归定义。

#### 状态、初始化与析构

```text
存储布局：
    begin_            end_                 capacity_
      | 活对象区间      | 未构造但可使用的存储    |

size():      空状态返回 0，否则 end_ - begin_
capacity():  空状态返回 0，否则 capacity_ - begin_

数量初始化(n, value):
    计算合法容量并分配
    用局部所有者保护新存储
    uninitialized_fill_n 构造 n 个对象
    全部成功后设置三个指针

范围初始化(first, last):
    前向及以上 → 计算数量，分配后逐个构造
    单遍输入 → 使用受局部 RAII 保护的临时容器，单遍追加后接管
    不能直接向未完成对象反复追加却依赖其析构清理失败

析构:
    若存储存在，销毁 [begin_, end_)
    使用关联资源和原容量归还存储
```

#### 容量增长与扩容提交

```text
检查新容量(新增数量):
    若 新增数量 > max_size - size → 抛尺寸异常
    required = size + 新增数量
    以不溢出的方式计算增长候选
    new_capacity >= required，且不超过 max_size

reserve(n):
    n <= capacity → 不操作
    分配 n 或合法的新容量
    在新存储拷贝构造全部旧元素
    成功后销毁旧元素并归还旧存储，再提交新状态
    失败则清理新状态，旧状态不变（按元素前提成立）

扩容插入(pos, n, value):
    n == 0 → 返回原位置，不做无意义指针运算
    保存位置偏移；空容器偏移视为 0
    申请至少 size+n 的新容量
    先构造插入段，保证 value 引用旧元素时来源未被破坏
    再构造原前段与后段
    守卫分别跟踪已构造区间
    全部成功后清理旧存储并提交
    返回 new_begin + 原偏移
```

基础阶段使用拷贝迁移；阶段七才按移动能力调整策略。不能仅将增长设为旧 size 的两倍，也不能只返回旧 pos。新状态构造成功前不修改旧存储的所有权。

#### 容量充足的数量插入

```text
insert(pos, n, value):
    n == 0 → 返回 pos
    若 value 可能来自本容器，先保存独立值
    若剩余容量不足 → 使用扩容插入
    否则 tail = end - pos，保存 old_end

    若 tail > n:
        将最后 n 个旧元素拷贝构造到尾部原始存储
        将中间已有对象向后赋值 n 个位置
        在 pos 起的 n 个已有对象中填充值
    否则:
        在 old_end 起构造 n-tail 个插入值
        将旧尾段构造到它的新位置
        给 pos 到 old_end 的旧对象赋插入值

    成功后更新 end 并返回 pos
    赋值失败时守卫销毁尾部新对象，保持旧对象区间有效
```

此路径的赋值可能改变已有值，一般只承诺适用的基本保证；失败后应避免 end 指向未构造位置。元素析构不抛是清理前提。

#### 删除、清空与调整大小

```text
erase(first, last):
    first == last → 返回 first
    将 [last, end) 向前赋值到 first
    得到 new_end
    Destroy(new_end, old_end)
    end = new_end
    返回 first

erase(pos):  删除 [pos, pos+1)，前提 pos 指向有效元素
pop_back():  在约定的非空前提下先 --end，再 Destroy(end)
clear():     销毁全部活对象，令 end = begin，保留容量

resize(n):
    n < size → 销毁尾部并更新 end
    n > size → 在尾部逐个默认构造，必要时扩容
resize(n, value):
    缩短同上，增长时构造指定值
```

默认扩展不应无意先创建一个 T() 再复制，避免把默认插入变成必须可拷贝的操作。零变化路径直接返回。

#### 拷贝、赋值、范围与资源

```text
拷贝构造(rhs):
    复制 rhs 的资源句柄，构造独立元素存储

拷贝赋值(rhs):
    自赋值可直接返回
    tmp = 拷贝构造(rhs)
    swap(tmp)                     // 句柄与存储一起交换
    tmp 析构时清理原目标资源

可选容量复用赋值:
    rhs.size > capacity → 临时副本替换
    rhs.size <= size → 给前段赋值，销毁多余尾段
    size < rhs.size <= capacity → 前 size 个赋值，只对新增尾部构造

范围 insert/assign:
    先依据迭代器能力分派
    输入或内部重叠来源可先收集进临时存储
    数量、容量、构造/赋值及返回值遵循已验证的路径
    空范围及时返回；assign 空输入则清空
```

数量与范围构造的重载问题不只是“两个参数造成歧义”；无约束的范围模板可能对整数提供更好的匹配，直到实例化函数体才报错。因此必须在候选选择阶段施加适当约束，并用实际调用验证。

#### 验证与 review

按接口逐项验证整数、非平凡及非默认可构造类型；检查 size/capacity、顺序、返回迭代器和生命周期。覆盖空范围、零数量、首中尾、容量充足/不足，以及“40 个元素插入 100 个”。逐位置注入构造或赋值失败，区分基本与强保证。

有状态池接入时明确资源是持有还是借用、客户与池的寿命关系，以及不同资源间赋值和 swap 的规则。存储最终必须交还原资源，不能只交换指针却丢失资源身份。

### 阶段七：vector 的现代 C++ 支持

#### 目标

在基础容器上完善移动语义、仅移动元素与原地构造，利用实际接口理解现代 C++ 机制。

#### 实现内容

- 实现移动构造和移动赋值，明确源状态、自移动以及有状态资源下能否直接接管存储。
- 实现 push_back(T&&) 和元素移动辅助操作，扩容按拷贝能力与移动异常属性选择策略。
- 将 Construct 扩展为参数包转发，实现 emplace_back；先容量内路径，再扩容路径。
- 审查算法、未初始化操作和修改接口是否无必要要求可拷贝或默认可构造，逐项支持仅移动类型的适用操作。
- 更新异常声明和保证：不可拷贝且移动可能抛出时，不无条件承诺保留原值的强保证。

#### 前置知识

值类别、右值引用、std::move、特殊成员生成规则、Rule of Five/Zero、unique_ptr、move_if_noexcept、引用折叠、转发引用、参数包和 std::forward。对推导不清楚时用短包装函数实验，理解后再接入库。

#### 现代化接口与迁移框架

```text
vector(vector&& rhs):
    按项目规则取得 rhs 的资源句柄与三个指针
    令 rhs 为空且保留可用资源关联

移动赋值(rhs):
    自移动按项目约定保持有效
    先用原资源清理目标旧存储
    再取得来源句柄与存储，来源置为有效空状态

Construct(p, args...):
    在 p 上执行 T(forward<Args>(args)...)

迁移(first, last, out):
    逐个以 move_if_noexcept(*first) 构造目标
    失败时清理目标成功部分
    不擅自销毁来源；由拥有者在提交成功后处理
```

#### 追加、转发与保证边界

```text
push_back(const T& value) → 保留左值复制行为
push_back(T&& value)      → 允许移动新元素

emplace_back(args...):
    有容量 → 直接转发构造，成功后 ++end
    无容量:
        分配新存储
        先构造尾部新元素，避免旧存储过早失效
        再按能力迁移旧元素，记录构造区间
        成功后清理旧状态并提交
```

move_if_noexcept 选择规则为移动不抛或不能拷贝时返回可移动引用，否则提供拷贝来源。若仅移动元素在迁移中途抛出，部分旧值可能已被改变，不能仅因新存储被清理就承诺强保证。新元素参数本身以右值引用指向旧元素时，也需要按实际语义单独审查，不能泛称所有内部引用都保持原值。

泛型 Swap 改用移动时，条件 noexcept 取决于实际的移动构造与移动赋值。vector 成员 swap 的不抛性质取决于句柄交换与指针交换，而不是元素的移动属性。

#### 验证与 review

使用 unique_ptr 元素验证追加、扩容和销毁；分别测试移动不抛、可拷贝但移动可能抛、仅移动且可能抛三类元素。检查整体移动与逐元素迁移次数，源状态和目标旧资源清理。验证左值/const 左值/右值转发、多参数构造、内部引用和失败路径，不把 emplace 等同于整个操作零移动。

### 阶段八：集成、回归与接口审查

#### 目标

形成可复现构建、可使用接口和有证据支持的能力说明，完成备份版本范围内的整体重建。

#### 实现内容

- 统一模块、实验、正式测试和诊断器构建入口；选项变化时可靠重编译，动态库也使用对应诊断选项。
- 运行简单与池后端的适用测试，审查资源关联和生命周期。
- 将旧问题清单转成回归条件，逐项说明已解决、不再适用、明确限制或仍待处理，不能因重写就全部标记修复。
- 整理公开接口、类型要求、返回语义、迭代器失效、异常保证、后端契约和使用示例。

#### 前置知识

头文件自包含、多编译单元、模板定义可见性、ODR、构建依赖和测试覆盖边界。

#### 构建与测试框架

```text
根 Makefile（实现目标）:
    all          构建当前正式模块
    learning     构建独立实验
    test         构建后运行自动测试，失败返回非零
    sanitize     独立目录重编译相关实现并运行 ASan/UBSan
    clean        仅清理明确的构建产物

pool/Makefile:
    编译后端源文件与明确的公开头文件
    不反向依赖 vector

test/Makefile:
    依赖公开头、后端库与测试辅助文件
    显式链接并设置运行时查找路径（使用动态库时）
```

#### 回归矩阵

| 模块 | 正常路径 | 边界与失败 |
| --- | --- | --- |
| 简单分配后端 | 分配/释放、类型适配 | 零尺寸、溢出、不支持对齐、失败 |
| 内存池 | 链表复用、补充、大小类别 | 空链表、类别边界、元数据失败、销毁 |
| 对象操作 | 构造、拷贝、填充、析构 | 第 N 次构造失败，只销毁成功部分 |
| 迭代器与算法 | 指针、标准迭代器、赋值结果 | 单遍输入、空区间、合法重叠、抛出赋值 |
| vector 容量 | reserve、追加、扩容 | 空状态、批量需求、溢出、内部引用 |
| vector 修改 | 插入、删除、assign、resize | 返回值、构造/赋值边界、失败后状态 |
| 现代接口 | 移动、unique_ptr、emplace | 源状态、自移动、迁移失败、转发选择 |
| 链接与资源 | 多源文件、两种后端 | ODR、资源寿命、原资源归还 |

验证记录区分“实际执行且通过”“失败”“未执行”。普通测试、诊断器、生命周期计数和静态 review 提供不同证据；任何一项都不能单独证明完整标准兼容。

#### 验证与 review

验收依据是接口与实现一致、关键路径可解释、测试可自动判定失败。预期编译失败的样例独立执行；assert 被关闭时不算有效检查。池内边界可能需要额外调试工具，诊断器无报告不单独证明正确。能独立指出错误承诺、生命周期风险或测试盲点，并提出验证办法，是学习目标的重要组成部分。

## 五、后续扩展

主体完成后再按兴趣选择并发池、任务队列、共享所有权与多态实验，或对照 C++17/20 学习 if constexpr、concepts、ranges 和 span。它们不改变第一轮以基础 vector 和内存池为核心的边界。

并发先采用完整操作的互斥保护，检查跨线程释放，使用独立 TSan 构建；分配器线程安全不代表容器可并发修改。性能基准在正确性稳定后建立，说明请求分布、重复次数、优化与内存占用，不预设内存池更快。
