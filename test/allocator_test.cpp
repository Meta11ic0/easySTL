#include <allocator.h>
#include <allocator.h>  // 验证重复包含不会重复定义。

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>

int main() {
  easystl::SimpleAllocator allocator;

  void* p = allocator.Allocate(sizeof(int), alignof(int));
  if (p == nullptr) return 1;
  const bool aligned = reinterpret_cast<std::uintptr_t>(p) % alignof(int) == 0;
  allocator.Deallocate(p, sizeof(int), alignof(int));
  if (!aligned) return 1;

  if (allocator.Allocate(0, alignof(int)) != nullptr) return 1;
  allocator.Deallocate(nullptr, 0, alignof(int));

  for (std::size_t alignment : {std::size_t{0}, std::size_t{3}}) {
    bool caught = false;
    try {
      p = allocator.Allocate(sizeof(int), alignment);
      allocator.Deallocate(p, sizeof(int), alignment);
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) return 1;
  }

  constexpr std::size_t alignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__ * 2;
  p = allocator.Allocate(alignment, alignment);
  if (p == nullptr) return 1;
  const bool over_aligned = reinterpret_cast<std::uintptr_t>(p) % alignment == 0;
  allocator.Deallocate(p, alignment, alignment);
  return over_aligned ? 0 : 1;
}
