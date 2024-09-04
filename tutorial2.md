# 深入浅出STL（二）：动手实现一个简单的分配器

## 1. 为什么要自定义分配器？

自定义分配器的主要目的是在特定场景下提高程序的性能，特别是在频繁的内存分配和释放操作中。通过更高效的内存管理，我们可以减少系统调用、降低内存碎片，从而提升程序的整体效率。在一些高性能应用场景中，标准的 `new` 操作符可能无法满足需求，这时就需要我们动手实现更为高效的分配器。

## 2. 实现一个简单的 `MallocAllocator`

### 2.1 `MallocAllocator` 的基本职责

`MallocAllocator` 是一个基础的分配器，它直接使用 C 库中的 `malloc` 和 `free` 来管理内存。尽管实现简单，但它为理解更复杂的分配器（如内存池）奠定了基础。下面是 `MallocAllocator` 的实现代码：

```cpp
class MallocAllocator {
 public:
  static void* Allocate(size_t size) {
    void *result = malloc(size);
    if (result == nullptr) { result = MallocInOom(size); }
    return result; 
  }
  
  static void Deallocate(void *obj, size_t /*size*/) {
    free(obj);
  }
  
  // 设置自定义的内存不足处理函数
  static void (*SetMallocHandler(void (*func)()))() {
    void (*old)() = CustomerOomHandler;
    CustomerOomHandler = func;
    return old;
  }

 private:
  // 内存不足时的处理函数
  static void* MallocInOom(size_t size);
  static void (*CustomerOomHandler)();
};
```

### 2.2 处理内存不足的情况

当 `malloc` 无法分配内存时，`MallocAllocator` 会调用用户自定义的内存不足处理函数 `CustomerOomHandler`，并尝试再次分配内存。下面是 `MallocInOom` 函数的实现：

```cpp
void* MallocAllocator::MallocInOom(size_t size) {
  void (*my_malloc_handler)();
  void *result;
  
  while (true) {
    my_malloc_handler = CustomerOomHandler;
    
    if (my_malloc_handler == nullptr) { 
      std::abort();  // 若未设置处理函数，直接终止程序
    }
    
    (*my_malloc_handler)();  // 调用用户定义的处理函数
    result = malloc(size);  // 再次尝试分配内存
    
    if (result) { return result; }
  }
}
```

## 3. 引入内存池技术

### 3.1 内存池的设计思路

内存池通过预先分配一大块内存来减少系统调用，并在这块内存中快速分配小块内存，从而提升分配速度并减少内存碎片。内存池特别适合小对象的频繁分配和释放，因为它避免了频繁的系统调用，提高了效率。

### 3.2 实现 `MemoryPoolAllocator`

`MemoryPoolAllocator` 基于 `MallocAllocator` 实现，使用内存池技术管理小内存块的分配和释放。对于较大的内存分配需求，仍然使用 `MallocAllocator`。以下是 `MemoryPoolAllocator` 的实现：

```cpp
class MemoryPoolAllocator {
 public:
  static void* Allocate(size_t size) {
    if (size > kMaxBytes) { 
      return MallocAllocator::Allocate(size); 
    }
    size_t index = GetFreelistIndex(size);
    if (freelist_[index].Empty()) {
      return ReFill(RoundUp(size));
    } else {
      return freelist_[index].Pop();
    }
  }

  static void Deallocate(void *obj, size_t size) {
    if (size > kMaxBytes) {
      MallocAllocator::Deallocate(obj, size);
      return;
    }
    
    freelist_[GetFreelistIndex(size)].Push(obj);
  }

 private:
  static size_t RoundUp(size_t bytes) { return ((bytes + kAlign - 1) & ~(kAlign - 1)); }
  static size_t GetFreelistIndex(size_t bytes) { return ((bytes + kAlign - 1) / kAlign - 1); } 
  static void* ReFill(size_t size);
  static char* ChunkAlloc(size_t size, size_t &chunknums);

  static MemoryPoolList freelist_[kFreeListNum];
  static char *freespacestart_;
  static char *freespaceend_;
  static size_t mallocoffset_;
};
```

### 3.3 内存池核心函数的实现

`ChunkAlloc` 和 `ReFill` 是内存池分配器的核心函数。`ChunkAlloc` 用于从系统中获取新的大块内存，并将其分割成小块供分配使用，而 `ReFill` 用于当 `freelist` 中的空闲链表为空时，填充新的内存块。

```cpp
char* MemoryPoolAllocator::ChunkAlloc(size_t size, size_t &chunknums) {
  char *result;
  size_t bytesneed = size * chunknums;
  size_t bytesleft = freespaceend_ - freespacestart_;
  
  if (bytesleft >= bytesneed) {
    result = freespacestart_;
    freespacestart_ += bytesneed;
    return result;
  } else if (bytesleft >= size) {
    chunknums = bytesleft / size;
    bytesneed = chunknums * size;
    result = freespacestart_;
    freespacestart_ += bytesneed;
    return result;
  } else {
    size_t bytesget = 2 * bytesneed + RoundUp(mallocoffset_ >> 4);
    
    if (bytesleft >= kAlign) {
      freelist_[GetFreelistIndex(bytesleft)].Push(freespacestart_);
    }
    
    freespacestart_ = (char *)malloc(bytesget);
    
    if (freespacestart_ == nullptr) {
      for (size_t i = size; i <= kMaxBytes; i += kAlign) {
        if (!freelist_[GetFreelistIndex(i)].Empty()) {
          freespacestart_ = (char*)freelist_[GetFreelistIndex(i)].Pop();
          freespaceend_ = freespacestart_ + i;
          return ChunkAlloc(size, chunknums);
        }
      }
      
      freespaceend_ = nullptr;
      freespacestart_ = (char *)MallocAllocator::Allocate(bytesget);
    }
    
    mallocoffset_ += bytesget;
    freespaceend_ = freespacestart_ + bytesget;
    return ChunkAlloc(size, chunknums);
  }
}

void* MemoryPoolAllocator::ReFill(size_t size) {
  size_t chunknums = 20;
  char *chunk = ChunkAlloc(size, chunknums);
  char *nextchunk = chunk + size;
  
  if (chunknums == 1) {
    return chunk;
  }
  
  for (int i = 1; i < chunknums; ++i) {
    freelist_[GetFreelistIndex(size)].Push(nextchunk);
    nextchunk += size;
  }
  
  return chunk;
}
```

## 4. 合并分配器接口

为了方便使用，我们可以将 `MallocAllocator` 和 `MemoryPoolAllocator` 合并成一个统一的接口，并根据分配的内存块大小选择最合适的分配器进行操作。

### 4.1 统一接口的实现

我们可以通过定义一个 `AllocatorWrapper` 模板类来实现这一点，该类可以根据宏定义灵活地选择使用哪种分配器：

```cpp
#define USEMALLOC
#ifdef USEMALLOC
using Allo = MemoryPoolAllocator;
#else
using Allo = MallocAllocator;
#endif

template<class T, class Allocator = easystl::Allo>
class AllocatorWrapper {
 public:
  static T* Allocate(size_t n) { return 0 == n ? 0 : (T*)Allocator::Allocate(n * sizeof(T)); }
  static T* Allocate(void) { return (T*)Allocator::Allocate(sizeof(T)); }
  static void Deallocate(T* p, size_t n) { if (0 != n) { Allocator::Deallocate(p, n * sizeof(T)); } }
  static void Deallocate(T* p) { Allocator::Deallocate(p, sizeof(T)); }
};
```

通过这种设计，我们不仅简化了分配器的使用，还能灵活地在不同的内存管理策略之间切换，以获得最佳性能。`AllocatorWrapper` 可以作为 STL 容器的默认分配器使用，充分利用自定义分配器的优势。

## 5. 分离内存管理与对象管理

在分配器的设计中，我们不仅要关注内存的分配和释放，还要关注对象的构造和析构。空间的配置和对象的构造其实是两个独立的过程：前者只负责内存的分配，而后者则负责对象的初始化。同样的，在释放内存之前，我们还需要先析构对象。

为了处理这一点，我们引入了 `Construct` 和 `Destroy` 函数，用于对象的构造和析构。

### 5.1 构造和析构的实现

下面是 `easystl` 命名空间下的一些基本构造和析构函数的实现：

```cpp
#ifndef EASYSTL_CONSTRUCTOR_H_
#define EASYSTL_CONSTRUCTOR_H_

#include <new>

namespace easystl {

// 默认构造函数
template<class T> 
inline void Construct(T* p) { 
  new(p) T(); 
}

// 带值构造函数
template<class T1, class T2>
inline void Construct(T1* p, const T2 & value) { 
  new(p) T1(value); 
}

// 析构函数
template<class T>
inline void Destroy(T* p) { 
  p->~T(); 
}

// 对区间内的对象进行析构
template<class ForwardIterator>
inline void Destroy(ForwardIterator first, ForwardIterator last) {
  for(; first != last; ++first) {
    Destroy(&*first);
  }
}

} // namespace easystl

#endif // EASYSTL_CONSTRUCTOR_H_
```

### 5.2 `Construct` 与 `Destroy` 的作用

- **`Construct` 函数**：该函数用于在已经分配好的内存空间上构造对象。这里使用了 **placement new** 语法，即在指定的内存地址上直接调用对象的构造函数。这允许我们在使用分配器分配的内存上手动构造对象。

- **`Destroy` 函数**：用于手动调用对象的析构函数。C++ 中的析构函数负责释放对象占用的资源，调用 `Destroy` 函数可以确保在释放内存之前正确地销毁对象。

通过 `Construct` 和 `Destroy` 函数，我们将内存的分配/释放与对象的构造/析构明确分开，这样的设计符合 C++ 标准库分配器的设计思想，也提高了代码的灵活性和可维护性。

### 5.3 结合分配器与构造器

在实际使用分配器时，我们可以通过以下步骤来分配和管理对象：

1. **分配内存**：首先使用分配器分配足够的内存空间，用于存放对象。
2. **构造对象**：在分配好的内存空间上调用 `Construct` 函数，构造对象。
3. **使用对象**：在构造完成后，对象可以正常使用。
4. **析构对象**：当对象不再需要时，调用 `Destroy` 函数析构对象。
5. **释放内存**：最后，使用分配器释放分配的内存。

这种分离的设计使得内存管理更加灵活，也更加符合 C++ 的 RAII（Resource Acquisition Is Initialization）原则。

## 结语

我们实现了一个基础的 `MallocAllocator`，并在此基础上引入了更复杂的 `MemoryPoolAllocator`。内存池技术的引入显著提高了内存分配的效率，特别是在处理大量小对象时。然后，我们通过一个统一的接口合并了两种分配器，使得在不同场景下能够灵活选择合适的分配器。最后，通过引入 `Construct` 和 `Destroy` 函数，我们进一步完善了分配器的功能，将内存的配置与对象的构造分开处理。这种设计不仅提高了内存管理的灵活性，还能更好地满足不同场景下的需求。


