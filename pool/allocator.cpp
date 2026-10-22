#include "allocator.h"

namespace easystl {

void (*MallocAllocator::CustomerOomHandler)() = nullptr;

void* MallocAllocator::MallocInOom(size_t size) {
  void (*my_malloc_handler)();
  void *result;
  while(true) {
    my_malloc_handler = CustomerOomHandler;
    // if CustomerOomHandler do not be setted
    // just exit
    if(nullptr==my_malloc_handler) {
      std::abort();
    };
    (*my_malloc_handler)();
    result = malloc(size);
    if(result) { return result; }
  }
}

void* MallocAllocator::ReallocInOom(void *obj, size_t size) {
  void (*my_malloc_handler)();
  void *result;
  while(true) {
    my_malloc_handler = CustomerOomHandler;
    // same as MallocInOom
    if(nullptr==my_malloc_handler) {
      std::abort();
    };
    (*my_malloc_handler)();
    result = realloc(obj, size);
    if(result) { return result; }
  }
}

char * MemoryPoolAllocator::freespacestart_ = nullptr;
char * MemoryPoolAllocator::freespaceend_ = nullptr;
size_t MemoryPoolAllocator::mallocoffset_ = 0;
MemoryPoolList MemoryPoolAllocator::freelist_[kFreeListNum];

char* MemoryPoolAllocator::ChunkAlloc(size_t size, size_t &chunknums) {
  char *result;
  size_t bytesneed = size * chunknums;
  size_t bytesleft =  freespaceend_ - freespacestart_;
  if (bytesleft >= bytesneed) {
    result  = freespacestart_;
    freespacestart_ += bytesneed;
    return result;
  }
  else if (bytesleft >= size) {
    chunknums = bytesleft/size;
    bytesneed = chunknums * size;
    result = freespacestart_;
    freespacestart_ += bytesneed;
    return result;
  }
  else {
    size_t bytesget = 2 * bytesneed + RoundUp(mallocoffset_ >> 4);
    if(bytesleft >= kAlign) {
        freelist_[GetFreelistIndex(bytesleft)].Push(freespacestart_);
    }
    freespacestart_ = (char *)malloc(bytesget);
    if (freespacestart_ == nullptr) {
      for (size_t i = size; i <= kMaxBytes; i += kAlign) {
        if(!freelist_[GetFreelistIndex(i)].Empty()) {
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
  if(chunknums == 1) {
    return chunk;
  }
  for (size_t i = 1; i < chunknums; ++i) {
    freelist_[GetFreelistIndex(size)].Push(nextchunk);
    nextchunk += size;
  }
  return chunk;
}

} // namespace easystl
