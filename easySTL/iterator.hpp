_Pragma("once")
#include <cstddef>

namespace easystl
{

class InputIteratorTag {};
class OutputIteratorTag {};
class ForwardIteratorTag : public InputIteratorTag {};
class BidirectionalIteratorTag : public ForwardIteratorTag {};
class RandomAccessIteratorTag : public BidirectionalIteratorTag {};

template<class Category,
         class T,
         class Distance = ptrdiff_t,
         class Pointer = T*,
         class Reference = T&>
class Iterator
{
  public:
    using iteratorCategory = Category;
    using valueType        = T;
    using pointer          = Pointer;
    using reference        = Reference;
    using differenceType   = Distance;
};

template<class Iterator>
class IteratorTraits
{
  public:
    using iteratorCategory = typename Iterator::iteratorCategory;
    using valueType        = typename Iterator::valueType;
    using pointer          = typename Iterator::pointer;
    using reference        = typename Iterator::reference;
    using differenceType   = typename Iterator::differenceType;
};

template<class T>
class IteratorTraits<T*>
{
  public:
    using iteratorCategory = RandomAccessIteratorTag;
    using valueType        = T;
    using pointer          = T*;
    using reference        = T&;
    using differenceType   = ptrdiff_t;
};

template<class T>
class IteratorTraits<const T*>
{
  public:
    using iteratorCategory = RandomAccessIteratorTag;
    using valueType        = T;
    using pointer          = const T*;
    using reference        = const T&;
    using differenceType   = ptrdiff_t;
};

template<class Iterator>
inline typename IteratorTraits<Iterator>::iteratorCategory
IteratorCategory(const Iterator&)
{
  typedef typename IteratorTraits<Iterator>::iteratorCategory Category;
  return Category();
}

template<class Iterator>
inline typename IteratorTraits<Iterator>::differenceType *
DistanceType(const Iterator&)
{
  return static_cast<typename IteratorTraits<Iterator>::differenceType *>(0);
}

template<class Iterator>
inline typename IteratorTraits<Iterator>::valueType *
ValueType(const Iterator&)
{
  return static_cast<typename IteratorTraits<Iterator>::valueType *>(0);
}

template<class InputIterator>
inline typename IteratorTraits<InputIterator>::differenceType 
__Distance(InputIterator first, InputIterator last, InputIteratorTag)
{
  typename IteratorTraits<InputIterator>::differenceType n = 0;
  while(first!=last)
  {
    ++first;
    ++n;
  } 
  return n;
}

template<class RandomAccessIterator>
inline typename IteratorTraits<RandomAccessIterator>::differenceType 
__Distance(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIteratorTag)
{
  return last - first;
}

template<class InputIterator>
inline typename IteratorTraits<InputIterator>::differenceType 
Distance(InputIterator first, InputIterator last)
{
  return __Distance(first, last, IteratorCategory(first));
}

template<class InputIterator, class Distance>
inline void __Advance(InputIterator &i, Distance n, InputIteratorTag)
{
  while (n--)
  {
    ++i;
  }
  
}

template<class BidirectionalIterator, class Distance>
inline void __Advance(BidirectionalIterator &i, Distance n, BidirectionalIteratorTag)
{
  if(n > 0)
  {
    while(n--)
    {
      ++i;
    }
  }
  else
  {
    while(n++)
    {
      --i;
    }
  }
}

template<class RandomAccessIterator, class Distance>
inline void __Advance(RandomAccessIterator &i, Distance n, RandomAccessIteratorTag)
{
  i += n;
}

template<class InputIterator, class Distance>
inline void Advance(InputIterator &i, Distance n)
{
  __Advance(i, n, IteratorCategory(i));
}

} // namespace easystl
