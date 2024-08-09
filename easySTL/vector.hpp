_Pragma("once")
#include "allocator.hpp"
#include "constructor.hpp"
#include "iterator.hpp"
#include "uninitialized.hpp"
#include "algo.hpp"

namespace easystl
{
template<class T, class Alloc = Allo>
class vector
{
    // Type alias
    public:
        using ValueType      = T;
        using Pointer        = ValueType*;
        using Iterator       = ValueType*;
        using Reference      = ValueType&;
        using SizeType       = size_t;
        using DifferenceType = ptrdiff_t;

    private:
        // Allocator
        using DataAllocator = AllocatorWrapper<T, Alloc>;
        // Flag
        Iterator m_begin;    // Stand for used memory head 
        Iterator m_end;      // Stand for used memory tail 
        Iterator m_capacity; // Stand for available memory tail 

        // Allocate memory and construct 
        // Memory size is n * sizeof(T)
        // Construct n variables of type T with values of value
        Iterator AllocateAndFill(SizeType n, const T& value)
        {
            Iterator res = DataAllocator::Allocate(n);
            uninitialized_fill_n(res, n, value);
            return res;
        }
        
        // Initialize flag
        void Initialize(SizeType n, const T& value)
        {
            m_begin = AllocateAndFill(n, value);
            m_end = m_begin + n;
            m_capacity = m_end;
        }

        // Destroy and Deallocate
        void DestroynDeallocate(Iterator first, Iterator last, SizeType n)
        {
            if(first)
            {
                Destroy(first, last);
                DataAllocator::Deallocate(first, n);
            }
        }

    public:
        // Basic operation
        Iterator begin() 
        {
            return m_begin;
        }

        Iterator end()
        {
            return m_end;
        }

        SizeType size() const
        {
            return static_cast<SizeType>(m_end - m_begin);
        }

        bool empty() const
        {
            return m_begin == m_end;
        }

        SizeType capacity() const
        {
            return static_cast<SizeType>(m_capacity - m_begin);
        }

        Reference operator[] (SizeType n)
        {
            return *(m_begin + n);
        }

        Reference front()
        {
            return *begin();
        }

        Reference back()
        {
            return *(m_end-1);
        }

        void pushback(const T& x)
        {
            if(m_end != m_capacity)
            {
                Construct(m_end, x);
                ++m_end;
            }
            else
            {
                insert_aux(m_end, x);
            }
        }

        void popback()
        {
            if(empty())
                return;
            --m_end;
            Destroy(m_end);
        }

        Iterator erase(Iterator pos)
        {
            if(pos + 1 != m_end)
                copy(pos + 1, m_end, pos);
            --m_end;
            Destroy(m_end);
            return pos;
        }

        void resize(SizeType newSize, const T& x)
        {
            if(newSize < size())
                erase(m_begin + newSize, m_end);
            else
                insert(m_end, newSize - size(), x);
        }

        void resize(SizeType newSize)
        {
            resize(newSize, T());
        }

        void clear()
        {
            erase(m_begin, m_end);
        }

        // insert
        void insert(Iterator pos, SizeType n, const T& x)
        {
            if(n == 0)
                return;
            const T xCpoy = x;
            if(SizeType(m_capacity -  m_end) >= n)
            {
                const SizeType elemsAfter = m_end - pos;
                Iterator oldEnd = m_end;
                if(elemsAfter > n)
                {
                    uninitialized_copy(m_end - n, m_end, m_end);
                    m_end += n;
                    copybackward(pos, oldEnd - n, oldEnd);
                    fill(pos, pos + n, xCpoy);
                }
                else
                {
                    uninitialized_fill_n(m_end, n - elemsAfter, xCpoy);
                    m_end += n - elemsAfter;
                    uninitialized_copy(pos, oldEnd, m_end);
                    m_end += elemsAfter;
                    fill(pos, oldEnd, xCpoy);
                }
            }
            else
            {
                const SizeType oldSize = max(size(), 16);
                const SizeType newSize = oldSize * 2;
                Iterator newBegin = DataAllocator::Allocate(newSize);
                Iterator newEnd = newBegin;
                try
                {
                    newEnd = uninitialized_copy(m_begin, pos, newBegin);
                    newEnd = uninitialized_fill_n(newEnd, n, xCpoy);
                    newEnd = uninitialized_copy(pos, m_end, newEnd);
                }
                catch(...)
                {
                    Destroy(newBegin, newEnd);
                    DataAllocator::Deallocate(newBegin, newSize);
                    throw;
                }

                DestroynDeallocate(m_begin, m_capacity, static_cast<SizeType>(m_capacity - m_begin));
                m_begin = newBegin;
                m_end = newEnd;
                m_capacity = m_begin + newSize;
            }
            
        }

        // Constructor
        vector() : m_begin(nullptr), m_end(nullptr), m_capacity(nullptr) {}
        
        vector(SizeType n, const T& value)
        {
            Initialize(n, value);
        }
        
        explicit vector(SizeType n)
        {
            Initialize(n, T());
        }

        // Destructor
        ~vector()
        {
            DestroynDeallocate(m_begin, m_end, static_cast<SizeType>(m_capacity - m_begin));
        }

        // copy constructor
        vector(const vector& other)
        {
            SizeType size = max(other.size(), static_cast<SizeType>(16));
            m_begin = DataAllocator::Allocate(size);
            m_end = m_begin;
            m_capacity = m_begin + size;
            uninitialized_copy(other.begin(), other.end(), m_begin);
        }

        // copy assignment operator
        vector& operator=(const vector& rhs)
        {
            if(*this != &rhs)
            {
                const SizeType rhsSize = rhs.size();
                if(rhsSize > capacity())
                {
                    vector tmp(rhs.begin(), rhs.end());
                    swap(tmp);
                }
                else if(size() >= rhsSize)
                {
                    auto i = copy(rhs.begin(), rhs.end(), m_begin);
                    DataAllocator::Destroy(i, m_end);
                    m_end = m_begin + rhsSize;
                }
                else
                {
                    copy(rhs.begin(), rhs.end(), m_begin);
                    uninitialized_copy(rhs.begin() + size(), rhs.end(), m_end);
                    m_end = m_begin + rhsSize;
                }
                
            }
            return *this;
        }

    private:
        void swap(const vector& rhs)
        {
            if(*this != &rhs)
            {
                easystl::swap(m_begin, rhs.m_begin);
                easystl::swap(m_end, rhs.m_end);
                easystl::swap(m_capacity, rhs.m_capacity);
            }
        }

        void insert_aux(Iterator pos, const T& x)
        {
            if(m_end != m_capacity)
            {
                Construct(m_end, x);
                ++m_end;
                copybackward(pos, m_end - 2, m_end - 1);
                *pos = x;
            }
            else
            {
                const SizeType newSize = max(size()*2, 16);
                Iterator newBegin = DataAllocator::Allocate(newSize);
                Iterator newEnd = newBegin;
                try
                {
                    newEnd = uninitialized_copy(m_begin, pos, newBegin);
                    Construct(newEnd, x);
                    ++newEnd;
                    newEnd = uninitialized_copy(pos, m_end, newEnd);
                }
                catch(...)
                {
                    Destroy(newBegin, newEnd);
                    DataAllocator::Deallocate(newBegin, newSize);
                    throw;
                }

                DestroynDeallocate(m_begin, m_capacity, static_cast<SizeType>(m_capacity - m_begin));
                m_begin = newBegin;
                m_end = newEnd;
                m_capacity = m_begin + newSize;
            }
        }

};

} // namespace easystl
