#ifndef EASYSTL_ALLOCATOR_H_
#define EASYSTL_ALLOCATOR_H_

#include <cstdlib>
#include <new>

namespace easystl{
// malloc allocator
// directly use malloc and free to manage memory
class MallocAllocator{
 public:
  static void* Allocate(size_t size) {
    void *result = malloc(size);
    if(0==result) { result = MallocInOom(size); }
    return result; 
  }
  static void Deallocate(void *obj, size_t /*size*/) {
    free(obj);
  }
  static void* Reallocate(void *obj, size_t /*oldsize*/, size_t newsize) {
    void *result = realloc(obj, newsize);
    if(0==result) { result = ReallocInOom(obj, newsize); }
    return result; 
  }
  // simulate set_new_handler in C++
  // customer can call this func to set
  // their own out-of-memory handler 
  static void (*SetMallocHandler(void (*func)()))() {
    void (*old)() = CustomerOomHandler;
    CustomerOomHandler = func;
    return old;
  }

 private:
  // funcs to be called when out of memory
  static void* MallocInOom(size_t size);
  static void* ReallocInOom(void *obj, size_t size);
  static void (*CustomerOomHandler)();
};

void (*MallocAllocator::CustomerOomHandler)() = nullptr;

void* MallocAllocator::MallocInOom(size_t size){
  void (*my_malloc_handler)();
  void *result;
  while(true){
    my_malloc_handler = CustomerOomHandler;
    // if CustomerOomHandler do not be setted 
    // just throw and exit 
    if(0==my_malloc_handler) { 
      throw std::bad_alloc();
    };
    (*my_malloc_handler)();
    result = malloc(size);
    if(result) { return result; }
  }
}

void* MallocAllocator::ReallocInOom(void *obj, size_t size){
  void (*my_malloc_handler)();
  void *result;
  while(true){
    my_malloc_handler = CustomerOomHandler;
    // same as MallocInOom
    if(0==my_malloc_handler) { 
      throw std::bad_alloc();
    };
    (*my_malloc_handler)();
    result = realloc(obj, size);
    if(result) { return result; }
  }
}

// memory pool class
// in order to save space
// every node will use itself 
// to be the pointer to the next
class MemoryPoolList
{
 public:
  bool Empty() const { return node_ == nullptr; }
  void*& GetNextNode(void *node) { return *static_cast<void**>(node); }
  void Push(void *node) {
    GetNextNode(node) = node_;
    node_ = node;
  }
  void* Pop(){
    void *result = node_;
    node_ = GetNextNode(result);
    return result;
  }

 private:
  void * node_ = nullptr;
};

// constexpr value
static constexpr int kAlign = 8;
static constexpr int kMaxBytes = 128;
static constexpr int kFreeListNum = kMaxBytes/kAlign;

// memory pool allocator
class MememoryPoolAllocator{
 public:
  static void* Allocate(size_t size) {
    if(size > size_t(kMaxBytes)) { return MallocAllocator::Allocate(size); };
    size_t index = GetFreelistIndex(size);
    if (freelist_[index].Empty()){
        return ReFill(RoundUp(size));
    }
    else{
        return freelist_[index].Pop();
    }
  }
  static void Deallocate(void *obj, size_t size) {
    if(size>kMaxBytes){
      MallocAllocator::Deallocate(obj, size);
      return ;
    }
    freelist_[GetFreelistIndex(size)].Push(obj);
  }
  static void* Reallocate(void *obj, size_t oldsize, size_t newsize) {
    if (RoundUp(newsize) == RoundUp(oldsize)) { 
      return obj; // No need to reallocate if sizes are equal
    }
    Deallocate(obj, oldsize);  // Free the old memory
    return Allocate(newsize);  // Allocate new memory
  }

 private:
  // align size to multiples of 8
  static size_t RoundUp(size_t bytes) { return ((bytes + kAlign - 1)&~(kAlign - 1)); }
  static size_t GetFreelistIndex(size_t bytes) { return ((bytes + kAlign - 1)/kAlign -1); } 
  // too big, define outside
  // refill freespace 
  static void* ReFill(size_t size);
  // get new chunk to freespace
  static char* ChunkAlloc(size_t size, int &numsofchunk);

  static MemoryPoolList freelist_[kFreeListNum];
  static char *freespacestart_;
  static char *freespaceend_;
  static size_t mallocoffset_;
};

char * MememoryPoolAllocator::freespacestart_ = nullptr;
char * MememoryPoolAllocator::freespaceend_ = nullptr;
size_t MememoryPoolAllocator::mallocoffset_ = 0;
MemoryPoolList MememoryPoolAllocator::freelist_[kFreeListNum];

char* MememoryPoolAllocator::ChunkAlloc(size_t size, int &numsofchunk) {
  char *result;
  size_t bytesneed = size * numsofchunk;
  size_t bytesleft =  freespaceend_ - freespacestart_;
  if (bytesleft >= bytesneed) {
    result  = freespacestart_;
    freespacestart_ += bytesneed;
    return result;
  }
  else if (bytesleft >= size) {
    numsofchunk = bytesleft/size;
    bytesneed = numsofchunk * size;
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
            return ChunkAlloc(size, numsofchunk);
        }
      }
      freespaceend_ = nullptr;
      freespacestart_ = (char *)MallocAllocator::Allocate(bytesget);
    }
    mallocoffset_ += bytesget;
    freespaceend_ = freespacestart_ + bytesget;
    return ChunkAlloc(size, numsofchunk);
  }
}

void* MememoryPoolAllocator::ReFill(size_t size) {
    int numsofchunk = 20;
    char *chunk = ChunkAlloc(size, numsofchunk);
    char *nextchunk = chunk + size;
    if(numsofchunk == 1) {
        return chunk;
    }
    for (int i = 1; i < numsofchunk; ++i) {
        freelist_[GetFreelistIndex(size)].Push(nextchunk);
        nextchunk += size;
    }
    return chunk;
}

#define USEMALLOC
#ifdef USEMALLOC
using Allo = MememoryPoolAllocator;
#else
using Allo = MallocAllocator;
#endif

template<class T, class Allocator = easystl::Allo>
class AllocatorWrapper {
 public:
  static T * Allocate(size_t n) { return 0 == n ? 0 : (T*)Allocator::Allocate(n*sizeof(T)); }
  static T * Allocate(void) { return (T*)Allocator::Allocate(sizeof(T)); }
  static void Deallocate(T * p, size_t n) { if(0 != n) { Allocator::Deallocate(p, n*sizeof(T)); } }
  static void Deallocate(T * p) { Allocator::Deallocate(p, sizeof(T)); }
};

} // namespace easystl

#endif // EASYSTL_ALLOCATOR_H_