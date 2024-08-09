_Pragma("once")
#include <iostream>
#include <cstddef>
#include <cstdlib>

namespace easystl
{
    
//first allocator
class MallocAllocator
{
    private:
        //oom: out of memory
        static void * oom_Malloc(size_t);
        static void * oom_Realloc(void *, size_t);
        static void (*oom_FirstHandler)();
    public:
        static void * Allocate(size_t n)
        {
            void * result = malloc(n);
            if(0 == result)
            {
                result = oom_Malloc(n);
            }
            return result;
        }

        static void Deallocate(void * p, size_t)
        {
            free(p);
        }

    static void *Reallocate(void *p, size_t /*oldsize*/, size_t newsize)
    {
        void *result = realloc(p, newsize);
        if (0 == result)
        {
            result = oom_Realloc(p, newsize);
        }
        return result;
    }

    static void (*set_malloc_handler(void (*f)()))()
    {
        void (*old)() = oom_FirstHandler;
        oom_FirstHandler = f;
        return old;
    }
};

void (*MallocAllocator::oom_FirstHandler)() = nullptr;

void * MallocAllocator::oom_Malloc(size_t n)
{
    void (*myMallocHandler)();
    void * result;

    while (true)
    {
        myMallocHandler = oom_FirstHandler;
        if(0 == myMallocHandler)
        {
            std::cerr << "out of memory!";
            std::abort();
        }
        (*myMallocHandler)();
        result = malloc(n);
        if(result)
            return result;
    }
}

void * MallocAllocator::oom_Realloc(void * p, size_t n)
{
    void (*myMallocHandler)();
    void * result;

    while (true)
    {
        myMallocHandler = oom_FirstHandler;
        if(0 == myMallocHandler)
        {
            std::cerr << "out of memory!";
            std::abort();
        }
        (*myMallocHandler)();
        result = realloc(p, n);
        if(result)
            return result;
    }
}

//second allocator
class FreeList
{
    private:
        void * m_freeList = nullptr;
    public:
        bool Empty() const
        {
            return m_freeList == nullptr;
        }
        void *& ObjNext(void * obj)
        {
            return *(void**)obj;
        }
        void Push(void * obj)
        {
            ObjNext(obj) = m_freeList;
            m_freeList = obj;
        }
        void * Pop()
        {
            void * result = m_freeList;
            m_freeList = ObjNext(result);
            return result;
        }
};

static const int ALIGN = 8;
static const int MAXBYTES = 128;
static const int FREELISTNUM = MAXBYTES/ALIGN;

class MemPoolAllocator
{
    private:
        static size_t RoundUp(size_t bytes)
        {
            return ((bytes + ALIGN - 1)&~(ALIGN - 1));
        }

        static size_t GetFreelistIndex(size_t bytes)
        {
            return ((bytes + ALIGN - 1)/ALIGN -1);
        }

        static void * ReFill(size_t n);
        static char * ChunkAlloc(size_t n, int & nobjs);

        static FreeList  m_freeList[FREELISTNUM];
        static char *  m_startFree;
        static char *  m_endFree;
        static size_t  m_heapsize;
    public:
        static void * Allocate(size_t n)
        {
            if(n > size_t(MAXBYTES))
            {
                return MallocAllocator::Allocate(n);
            }

        size_t index = GetFreelistIndex(n);
        if (m_freeList[index].Empty())
        {
            return ReFill(RoundUp(n));
        }
        else
        {
            return m_freeList[index].Pop();
        }
    }

        static void Deallocate(void * obj, size_t n)
        {
            if(n > MAXBYTES)
            {
                MallocAllocator::Deallocate(obj, n);
                return;
            }

            m_freeList[GetFreelistIndex(n)].Push(obj);
        }

        static void * Reallocate(void * obj, size_t oldsize, size_t newsize)
        {

            if(newsize > size_t(MAXBYTES) && oldsize > size_t(MAXBYTES))
            {
                return MallocAllocator::Reallocate(obj, oldsize, newsize);
            }

            if (RoundUp(newsize) == RoundUp(oldsize))
            {
                return obj;
            }

            void * result = Allocate(newsize);
            size_t copySize = newsize > oldsize ? oldsize : newsize;
            memcpy(result, obj, copySize);
            Deallocate(obj, oldsize);

            return result;
        }
};

char * MemPoolAllocator::m_startFree = nullptr;
char * MemPoolAllocator::m_endFree = nullptr;
size_t MemPoolAllocator::m_heapsize = 0;
FreeList MemPoolAllocator::m_freeList[FREELISTNUM];

char * MemPoolAllocator::ChunkAlloc(size_t n, int & nobjs)
{
    char * result;
    size_t bytesNeed = n * nobjs;
    size_t bytesLeft =  m_endFree - m_startFree;

    if (bytesLeft >= bytesNeed)
    {
        result  = m_startFree;
        m_startFree += bytesNeed;
        return result;
    }
    else if (bytesLeft >= n)
    {
        nobjs = bytesLeft/n;
        bytesNeed = nobjs * n;
        result = m_startFree;
        m_startFree += bytesNeed;
        return result;
    }
    else
    {
        size_t bytesGet = 2 * bytesNeed + RoundUp(m_heapsize >> 4);
        if(bytesLeft >= ALIGN)
        {
            m_freeList[GetFreelistIndex(bytesLeft)].Push(m_startFree);
        }

        m_startFree = (char *)malloc(bytesGet);
        if (m_startFree == nullptr)
        {
            for (size_t i = n; i <= MAXBYTES; i += ALIGN)
            {
                if(!m_freeList[GetFreelistIndex(i)].Empty())
                {
                    m_startFree = (char*)m_freeList[GetFreelistIndex(i)].Pop();
                    m_endFree = m_startFree + i;
                    return ChunkAlloc(n, nobjs);
                }
            }
            m_endFree = nullptr;
            m_startFree = (char *)MallocAllocator::Allocate(bytesGet);
        }
        m_heapsize += bytesGet;
        m_endFree = m_startFree + bytesGet;
        return ChunkAlloc(n, nobjs);
    }
}

void * MemPoolAllocator::ReFill(size_t n)
{
    int nobjs = 20;
    char * chunk = ChunkAlloc(n, nobjs);
    char * nextChunk = chunk + n;

    if(nobjs == 1)
    {
        return chunk;
    }

    for (int i = 1; i < nobjs; ++i)
    {
        m_freeList[GetFreelistIndex(n)].Push(nextChunk);
        nextChunk += n;
    }
    return chunk;
}

#define USEMALLOC
#ifdef USEMALLOC
using Allo = MemPoolAllocator;
#else
using Allo = MallocAllocator;
#endif

template<class T, class Allocator = easystl::Allo>
class AllocatorWrapper
{
    public:
        static T * Allocate(size_t n)
        {
            return 0 == n ? 0 : (T*)Allocator::Allocate(n*sizeof(T));
        }

        static T * Allocate(void)
        {
            return (T*)Allocator::Allocate(sizeof(T));
        }

        static void Deallocate(T * p, size_t n)
        {
            if(0 != n)
            {
                Allocator::Deallocate(p, n*sizeof(T));
            }
        }

        static void Deallocate(T * p)
        {
            Allocator::Deallocate(p, sizeof(T));
        }
};

} // namespace easystl