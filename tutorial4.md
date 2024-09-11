# 深入浅出STL（四）：构建自己的 `vector`

在现代C++编程中，`vector` 是最常用的容器之一。它不仅提供了动态数组的功能，还能自动管理内存，并提供高效的访问和修改操作。然而，对于很多开发者来说，`vector` 的内部实现仍然是个谜。通过自己动手实现一个 `vector`，我们可以深入理解其设计思想，进而提高编程能力。在本教程中，我们将带你一步步实现一个自定义的 `vector`，并深入探讨其设计和实现细节。通过这个过程，你将更好地理解 `vector` 的内部机制，并掌握实现动态数组的关键技术。

## 1. 什么是 `vector`？

在C++中，`vector` 是一种动态数组，它的大小可以在运行时动态调整。与传统的静态数组相比，`vector` 具有以下几个显著优势：

- **连续内存存储**：`vector` 内部使用一块连续的内存来存储数据，这使得元素的访问非常高效，特别是当你需要随机访问某个元素时，通过索引可以直接定位到该元素。

- **动态扩展**：当 `vector` 的容量不足时，它会自动扩展以容纳更多的元素。这种扩展通常是成倍扩展的，以减少频繁扩容的开销。

- **高效访问**：由于 `vector` 使用连续的内存存储数据，它能够像数组一样通过 `operator[]` 或 `at` 函数高效地访问任意元素。同时，`vector` 也提供了边界检查功能，确保访问安全。

- **自动内存管理**：`vector` 自动处理内存的分配与释放，避免了手动管理内存可能带来的错误。你不需要担心内存泄漏或未初始化的内存问题，`vector` 会在合适的时机分配和释放内存。

此外，`vector` 还支持复杂的数据结构操作，如插入、删除、排序等，并在内部实现了对这些操作的高效支持。

## 2. `vector` 的基本操作

在构建自己的 `vector` 之前，理解其基本操作是必要的。下面我们将详细介绍 `vector` 的核心操作。

### 2.1 添加元素

- **push_back**：向 `vector` 的末尾添加一个元素。这是 `vector` 中最常用的操作之一。当 `vector` 的容量不足以容纳新元素时，它会自动分配更多的内存来扩展。

  ```cpp
  std::vector<int> vec;
  vec.push_back(10);
  vec.push_back(20);
  ```

- **insert**：在 `vector` 的指定位置插入一个或多个元素。与 `push_back` 不同，`insert` 会影响插入点后的所有元素，可能导致这些元素在内存中移动。

  ```cpp
  std::vector<int> vec = {1, 2, 3};
  vec.insert(vec.begin() + 1, 10);  // 在第二个位置插入10
  ```

### 2.2 删除元素

- **pop_back**：从 `vector` 的末尾删除一个元素。这是 `vector` 中与 `push_back` 对应的操作，用于移除最后一个元素。

  ```cpp
  std::vector<int> vec = {1, 2, 3};
  vec.pop_back();  // 删除最后一个元素3
  ```

- **erase**：删除 `vector` 中指定位置的一个或多个元素。这个操作会影响到删除位置后的所有元素，导致它们在内存中的位置发生变化。

  ```cpp
  std::vector<int> vec = {1, 2, 3, 4};
  vec.erase(vec.begin() + 1);  // 删除第二个元素2
  ```

### 2.3 访问元素

- **operator[]**：通过索引直接访问元素。这个操作符不进行边界检查，因此使用时需要确保索引在有效范围内。

  ```cpp
  std::vector<int> vec = {1, 2, 3};
  int value = vec[1];  // 访问第二个元素
  ```

- **at**：通过索引访问元素，并进行边界检查。如果索引超出范围，会抛出 `std::out_of_range` 异常。

  ```cpp
  try {
      std::vector<int> vec = {1, 2, 3};
      int value = vec.at(10);  // 超出范围，抛出异常
  } catch (const std::out_of_range& e) {
      std::cerr << "Out of range: " << e.what() << std::endl;
  }
  ```

## 3. `vector` 的内存管理

`vector` 作为动态数组，内存管理是其核心功能之一。我们需要了解 `vector` 是如何管理其内部内存的。

### 3.1 动态内存分配

当 `vector` 中的元素超出当前容量时，`vector` 需要分配新的内存，并将现有元素复制到新的内存空间中。这一过程叫做“扩容”。通常情况下，`vector` 会成倍扩展其容量，以减少频繁扩容的成本。由于 `vector` 使用的是连续内存，因此扩容时需要将所有元素从旧的内存块复制到新的、更大的内存块中。

### 3.2 容量与大小管理

- **size**：`vector` 当前保存的元素个数。可以通过 `size()` 函数获取。
  
  ```cpp
  std::vector<int> vec = {1, 2, 3};
  std::cout << "Size: " << vec.size() << std::endl;  // 输出 3
  ```

- **capacity**：`vector` 在当前内存分配中可以容纳的最大元素个数。通过 `capacity()` 函数可以查看。这通常比 `size` 大，因为 `vector` 会预留一些额外的空间来减少频繁的内存分配。

  ```cpp
  std::cout << "Capacity: " << vec.capacity() << std::endl;  // 可能输出大于3的值
  ```

- **reserve**：用于预先分配内存，避免在未来的插入操作中频繁扩容。`reserve()` 函数可以将 `vector` 的容量设置为至少指定的值，从而在插入大量元素前减少扩容的次数。
  
  ```cpp
  vec.reserve(100);  // 将容量至少扩展到100
  ```

- **shrink_to_fit**：减少 `vector` 的容量，使其与当前大小相匹配。调用 `shrink_to_fit()` 之后，`vector` 将尽可能释放未使用的内存。这在减少内存占用时非常有用。

  ```cpp
  vec.shrink_to_fit();  // 释放多余的内存
  ```

## 4. 实现一个简单的 `vector` 类

现在，我们开始动手实现一个自己的 `vector`。这个实现会包含最基础的功能，帮助我们理解 `vector` 的核心机制。

### 4.1 类的定义与基本成员

首先，我们定义一个类模板，并声明一些必要的成员变量。

```cpp
template<class T, class Alloc = AllocatorWrapper<T, Alloc>> // 使用自定义分配器
class vector {
private:
    T* begin_;    // 指向内存的起始位置
    T* end_;      // 指向内存的末尾位置
    T* capacity_; // 指向内存的容量末尾位置

public:
    // 构造函数、析构函数等成员函数的声明
};
```

### 4.2 构造函数与析构函数

构造函数用于初始化 `vector`，而析构函数用于清理资源。

- **默认构造函数**：初始化一个空的 `vector`。

  ```cpp
  vector() noexcept : begin_(nullptr), end_(nullptr), capacity_(nullptr) {}
  ```

- **带大小和初始值的构造函数**：创建一个指定大小的 `vector`，并用初始值填充。

  ```cpp
  vector(size_type len, const T& value) noexcept {
      NumsInit(len, value); // 初始化函数，后文详述
  }
  ```

- **拷贝构造函数**：实现深拷贝，确保 `vector` 在复制时行为正确。

  ```cpp
  vector(const vector& other) noexcept {
      RangeInit(other.begin_, other.end_); // 范围初始化函数，后文详述
  }
  ```

- **析构函数**：释放动态分配的内存，避免内存泄漏。

  ```cpp
  ~vector() noexcept {
      DestroynDeallocate(begin_, end_, static_cast<size_type>(capacity_ - begin_)); // 资源清理函数，后文详述
  }
  ```

### 4.3 内存管理函数

为了高效管理内存，增加代码可读性以及减少重复代码，我们需要实现 `NumsInit` 、`RangeInit` 和 `DestroynDeallocate` 函数。

- **NumsInit**：初始化指定数量的元素，并填充初始值。

  ```cpp
  void NumsInit(size_type n, const T& value) noexcept {
      size_type initsize = easystl::Max(static_cast<size_type>(n), static_cast<size_type>(16)); // 起码分配16个
      begin_ = DataAllocator::Allocate(initsize);
      end_ = easystl::uninitialized_fill_n(begin_, n, value); // 在未初始化的内存上构造对象
      capacity_ = begin_ + initsize;
  }
  ```

- **RangeInit**：从一个迭代器范围初始化 `vector`。

  ```cpp
  template <class Iter>
  void RangeInit(Iter first, Iter last) noexcept {
      size_type initsize = easystl::Max(static_cast<size_type>(last - first), static_cast<size_type>(16));
      begin_ = DataAllocator::Allocate(initsize);
      end_ = easystl::uninitialized_copy(first, last, begin_); // 在未初始化的内存上构造对象
      capacity_ = begin_ + initsize;
  }
  ```

- **DestroynDeallocate**：销毁并释放内存。

  ```cpp
  void DestroynDeallocate(iterator first, iterator last, size_type len) noexcept {
      if (first) {
          Destroy(first, last); // 销毁对象
          DataAllocator::Deallocate(first, len); // 释放内存
      }
  }
  ```

### 4.4 全局初始化辅助函数

为了更好地管理内存，我们需要实现一些全局辅助函数。这些函数用于在未初始化的内存上构造对象。主要包括 `uninitialized_copy`、`uninitialized_fill` 和 `uninitialized_fill_n`。

- **uninitialized_copy**：从输入迭代器范围 `[first, last)` 复制元素到未初始化的内存区域 `[result, result + (last - first))`。

  ```cpp
  template <class InputIter, class ForwardIter>
  ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result) {
      auto current = result;
      try {
          for (; first != last; ++first, ++current) {
              easystl::Construct(&*current, *first); // 在当前位置构造元素
          }
      }
      catch (...) {
          easystl::Destroy(result, current); // 如果发生异常，销毁已经构造的元素
          std::abort(); // 终止程序
      }
      return current; // 返回最后一个构造元素的下一个位置
  }
  ```

  - **参数说明**：
    - `first`：输入范围的起始迭代器。
    - `last`：输入范围的结束迭代器。
    - `result`：目标内存区域的起始迭代器。

  - **工作流程**：
    1. 从 `first` 开始，逐个元素复制到 `result` 指向的未初始化内存区域。
    2. 如果在复制过程中发生异常，销毁已经构造的元素并终止程序。

- **uninitialized_fill**：在未初始化的内存区域 `[first, last)` 构造并填充指定的值。

  ```cpp
  template <class ForwardIter, class T>
  void uninitialized_fill(ForwardIter first, ForwardIter last, const T& value) {
      auto current = first;
      try {
          for (; current != last; ++current) {
              easystl::Construct(&*current, value); // 在当前位置构造元素
          }
      }
      catch (...) {
          easystl::Destroy(first, current); // 如果发生异常，销毁已经构造的元素
          std::abort(); // 终止程序
      }
  }
  ```

  - **参数说明**：
    - `first`：目标内存区域的起始迭代器。
    - `last`：目标内存区域的结束迭代器。
    - `value`：要填充的值。

  - **工作流程**：
    1. 从 `first` 开始，在未初始化的内存区域构造并填充 `value`。
    2. 如果在填充过程中发生异常，销毁已经构造的元素并终止程序。

- **uninitialized_fill_n**：在未初始化的内存区域 `[first, first + n)` 构造并填充指定数量的值。

  ```cpp
  template <class ForwardIter, class Size, class T>
  ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value) {
      auto current = first;
      try {
          for (; n > 0; --n, ++current) {
              easystl::Construct(&*current, value); // 在当前位置构造元素
          }
      }
      catch (...) {
          easystl::Destroy(first, current); // 如果发生异常，销毁已经构造的元素
          std::abort(); // 终止程序
      }
      return current; // 返回最后一个构造元素的下一个位置
  }
  ```

  - **参数说明**：
    - `first`：目标内存区域的起始迭代器。
    - `n`：要构造的元素数量。
    - `value`：要填充的值。

  - **工作流程**：
    1. 从 `first` 开始，在未初始化的内存区域构造并填充 `n` 个 `value`。
    2. 如果在填充过程中发生异常，销毁已经构造的元素并终止程序。

这些全局初始化辅助函数的核心思想是在未初始化的内存区域上构造元素，并且在发生异常时能够安全地清理已经构造的元素，避免内存泄漏。这些函数是实现 `vector` 类内存管理的重要工具。

### 4.5 基本操作的实现

接下来，我们实现 `vector` 的一些基本操作，如 `push_back`、`pop_back` 以及 `operator[]`。

- **push_back**：将新元素添加到 `vector` 的末尾。如果容量不足，需要先扩容。

  ```cpp
  void push_back(const T& x) noexcept {
      if (end_ != capacity_) {
          Construct(end_, x); // 在末尾构造新元素
          ++end_;
      } else {
          InsertAux(end_, 1, x); // 如果容量不足，进行扩容后再插入，后文详述
      }
  }
  ```

- **pop_back**：移除 `vector` 末尾的元素。

  ```cpp
  void pop_back() noexcept {
      if (empty()) { return; }
      --end_;
      Destroy(end_); // 销毁末尾元素
  }
  ```

- **operator[]** 和 **at**：实现元素的直接访问。

  ```cpp
  reference operator[] (size_type n) noexcept { return *(begin_ + n); }
  reference at(size_type n) noexcept {
      if (n >= size()) throw std::out_of_range("Index out of range");
      return *(begin_ + n);
  }
  ```

### 4.6 `vector`的迭代器

`vector` 是基于连续内存存储数据的容器，因此它的迭代器可以直接使用**指针**实现。因为迭代器的作用是提供对 `vector` 中元素的直接访问，并支持各种迭代操作。还有跟上一章的`iterator_traits`一样，我们同样需要针对`vector`有一个类似实现。用于我们可以通过`vector`对象获取里面存储的元素的类型、迭代器的类型等内容。

#### 4.6.1 迭代器类型定义

在 `vector` 类中，迭代器类型通常定义为指向元素的指针，并且可以定义 `const_iterator` 来表示只读迭代器。

```cpp
template<class T, class Alloc = AllocatorWrapper<T, Alloc>>
class vector {
public:
    // 类型别名定义
    using value_type      = T;
    using pointer         = T*;
    using iterator        = T*;
    using const_iterator  = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    // 其他内容
};
```

- **`iterator`**：定义为 `T*`，它是一个普通指针，能够访问和修改 `vector` 中的元素。
- **`const_iterator`**：定义为 `const T*`，它是一个只读指针，只能访问元素而不能修改。

#### 4.6.2 基本迭代器操作

通常在 `vector` 中，我们会提供以下几个基本的迭代器操作方法：

- **`begin()`**：返回指向第一个元素的迭代器。
- **`end()`**：返回指向最后一个元素后面的迭代器（不指向任何有效元素）。
- **`cbegin()`** 和 **`cend()`**：分别返回 `const_iterator` 类型的 `begin` 和 `end`，用于在 `const vector` 对象上进行迭代。

```cpp
iterator begin() noexcept { return begin_; }
iterator end() noexcept { return end_; }
const_iterator begin() const noexcept { return begin_; }
const_iterator end() const noexcept { return end_; }
const_iterator cbegin() const noexcept { return begin_; }
const_iterator cend() const noexcept { return end_; }
```

- **`begin()`** 和 **`end()`** 返回普通迭代器，允许修改元素。
- **`cbegin()`** 和 **`cend()`** 返回 `const_iterator`，只允许读取元素。

### 4.7 扩容机制： `InsertAux` 的实现

`vector` 的扩容是其核心功能之一，因为它的大小是动态调整的。当添加新元素时，如果 `vector` 当前的容量不足以容纳新元素，它需要进行扩容。扩容过程涉及到分配新的更大容量的内存，将原有元素复制到新内存中，并释放旧的内存。主要起作用的是一个辅助函数 `InsertAux`，在 `vector` 的末尾插入元素并在必要时进行扩容。其基本工作原理是：如果当前容量足够，则直接插入元素；否则，扩容后再插入元素。

```cpp
void InsertAux(iterator pos, size_type nums, const T& x) noexcept {
    const size_type newsize = Max(size()*2, static_cast<size_type>(16)); // 新容量通常为当前容量的两倍
    iterator newbegin = DataAllocator::Allocate(newsize); // 分配新内存
    iterator newend = newbegin;

    newend = uninitialized_copy(begin_, pos, newbegin); // 复制原有数据到新内存
    newend = uninitialized_fill_n(newend, nums, x); // 插入新元素
    newend = uninitialized_copy(pos, end_, newend); // 复制剩余数据到新内存

    DestroynDeallocate(begin_, end_, static_cast<size_type>(capacity_ - begin_)); // 释放旧内存
    begin_ = newbegin;
    end_ = newend;
    capacity_ = begin_ + newsize;
}
```

### 5. 特别注意

在C++模板编程中，**模板重载冲突**是一个常见问题，尤其是在使用构造函数时。当设计`vector`类时，构造函数可能接受两个迭代器参数用于范围初始化，也可能接受一个长度和一个初始值进行初始化。这时编译器可能无法区分这两个重载，因为它们的参数类型相同，导致冲突。

### 5.1 模板重载冲突

例如，以下代码中的两个构造函数会产生冲突：

```cpp
class vector {
 public:
  // 接受长度和初值的构造函数
  explicit vector(size_type len, const T& value) noexcept { NumsInit(len, value); }

  // 接受两个迭代器的构造函数
  template<class Iterator>
  vector(Iterator first, Iterator last) noexcept {
    RangeInit(first, last);
  }
};

// 示例
int main() {
  vector<int> v1(10, 5);                 // 编译器疑惑调用哪个构造函数
  vector<int> v2(v1.begin(), v1.end());  // 编译器疑惑调用哪个构造函数
}
```

两个构造函数的参数形式相同（均为两个参数），编译器无法确定它们的具体用途。例如，当传递两个整数时，编译器无法区分这些整数是表示长度和初值，还是表示迭代器范围。这种情况会导致**模板重载歧义**，从而出现编译错误。为了解决这个问题，我们可以借助模板元编程与SFINAE机制。

### 5.2 基础的`IteratorTraits`实现

我们首先需要一种机制来区分迭代器和非迭代器。`IteratorTraits`提供了一种方法来提取类型的迭代器特征。首先，我们为所有非迭代器类型提供一个基础实现：

```cpp
template<class T, class = void>
class IteratorTraits {};
```

- 这个实现适用于所有不符合迭代器特征的类型，**它提供默认的空实现，防止在处理非迭代器类型时出现编译错误**。

### 5.3 偏特化的`IteratorTraits`实现

接下来，我们通过**偏特化**为真正的迭代器类型提供特定实现：

```cpp
template<class T>
class IteratorTraits<T, typename std::void_t<typename T::iterator_category>> {
 public:
  using IteratorCategory = typename T::iterator_category;
};
```

- 这里使用`void_t`是为了检测某个类型是否拥有`iterator_category`。`void_t`的原理是，当某个类型不包含所需的成员（如`iterator_category`），模板替换会失败，从而触发SFINAE机制，编译器不会报错，而是会选择基础模板版本。这使得我们可以有效区分迭代器和非迭代器类型。
- `std::void_t` 是 C++17 引入的一个元编程工具，用于检测某些类型的有效性。它通过将任意数量的类型参数替换为 `void`，帮助我们检查这些类型是否存在而不产生编译错误。

### 5.4 `IsIterator`的实现

为了更高效地判断一个类型是否为迭代器，我们可以使用模板元编程结合SFINAE机制来实现`IsIterator`：

```cpp
template <class T>
class IsIterator {
 private:
  // 非迭代器类型匹配
  template<class U>
  static std::false_type test(...);

  // 迭代器类型匹配
  template<class U>
  static auto test(int) -> decltype(typename IteratorTraits<U>::IteratorCategory(), std::true_type());

 public:
  // 判断是否为迭代器类型
  static const bool value = decltype(test<T>(0))::value;
};
```

**代码解读**：
- `test(int)`分支通过`decltype`机制判断类型是否包含`IteratorCategory`，即是否为迭代器。如果是迭代器，返回`std::true_type`。
- `test(...)`分支是一个捕获所有其他情况的泛化版本，当类型不是迭代器时返回`std::false_type`。
- 最终，`decltype(test<T>(0))::value`判断类型`T`是否为迭代器。
- `test(int)` 是一种优先匹配的重载，它用于检测 `IteratorCategory` 是否存在。而 `test(...)` 是一个“兜底”方案，用于捕获所有其他情况。当类型不包含 `IteratorCategory` 时，`test(...)` 会被调用。

### 5.5 使用`std::enable_if`结合`IsIterator`

在构造函数中，我们可以利用`std::enable_if`和`IsIterator`来区分迭代器和其他类型，确保只有传入迭代器时，才会实例化迭代器构造函数：

```cpp
template<class Iterator, typename std::enable_if<IsIterator<Iterator>::value, int>::type = 0>
vector(Iterator first, Iterator last) noexcept {
  RangeInit(first, last);
}
```

- `std::enable_if<IsIterator<Iterator>::value, int>::type = 0`确保了只有在`Iterator`是迭代器类型时，才会启用这个重载版本，从而避免与其他构造函数产生冲突。

### 5.6 SFINAE机制

这个方案依赖于C++中的**SFINAE**（Substitution Failure Is Not An Error）机制。SFINAE意味着当模板参数替换失败时，编译器不会报错，而是选择另一个匹配的模板重载。这使得我们可以编写出更灵活、通用的代码，并有效避免模板重载冲突。

通过这些技巧，我们可以确保`vector`类的构造函数在不同参数类型下都能正确匹配：

```cpp
int main() {
  vector<int> v1(10, 5);                 // 匹配长度和初值的构造函数
  vector<int> v2(v1.begin(), v1.end());  // 匹配迭代器范围的构造函数
}
```
