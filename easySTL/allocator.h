#ifndef EASYSTL_ALLOCATOR_H_
#define EASYSTL_ALLOCATOR_H_

// <allocator.h> 是 pool 的头文件。编译时 -I pool/include 必须写在 -I easySTL 前面，
// 否则这里会包含到本文件自己。
#include <allocator.h>

namespace easystl {

#define USEMALLOC
#ifdef USEMALLOC
using Allo = MemoryPoolAllocator;
#else
using Allo = MallocAllocator;
#endif

template<class T, class Allocator = Allo>
class AllocatorWrapper {
 public:
  static T * Allocate(size_t n) { return 0 == n ? 0 : (T*)Allocator::Allocate(n*sizeof(T)); }
  static T * Allocate(void) { return (T*)Allocator::Allocate(sizeof(T)); }
  static void Deallocate(T * p, size_t n) { if(0 != n) { Allocator::Deallocate(p, n*sizeof(T)); } }
  static void Deallocate(T * p) { Allocator::Deallocate(p, sizeof(T)); }
};

} // namespace easystl

#endif // EASYSTL_ALLOCATOR_H_
