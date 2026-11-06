#ifndef EASYSTL_ALLOCATOR_H
#define EASYSTL_ALLOCATOR_H

#include <cstddef>
#include <new>
#include <stdexcept>

namespace easystl {

class SimpleAllocator {
 public:
  void* Allocate(std::size_t bytes, std::size_t alignment) {
    if (bytes == 0) return nullptr;
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
      throw std::invalid_argument("invalid alignment");
    }
    if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      return ::operator new(bytes, std::align_val_t{alignment});
    }
    return ::operator new(bytes);
  }

  void Deallocate(void* p, std::size_t, std::size_t alignment) noexcept {
    if (p == nullptr) return;
    if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
      ::operator delete(p, std::align_val_t{alignment});
    } else {
      ::operator delete(p);
    }
  }
};

}  // namespace easystl

#endif  // EASYSTL_ALLOCATOR_H
