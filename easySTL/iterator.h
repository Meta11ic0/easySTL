#ifndef EASYSTL_ITERATOR_H_
#define EASYSTL_ITERATOR_H_

#include <cstddef>

namespace easystl {

class TrueType {
 public:
  static const bool value = true;
};

class FalseType {
 public:
  static const bool value = false;
};

template<typename...> using void_t = void;

class InputIteratorTag {};
class OutputIteratorTag {};
class ForwardIteratorTag : public InputIteratorTag {};
class BidirectionalIteratorTag : public ForwardIteratorTag {};
class RandomAccessIteratorTag : public BidirectionalIteratorTag {};

template<class Category,
         class T,
         class distance = ptrdiff_t,
         class pointer = T*,
         class reference = T&>
class Iterator {
  public:
    using IteratorCategory = Category;
    using ValueType        = T;
    using Pointer          = pointer;
    using Reference        = reference;
    using DifferenceType   = distance;
};

template<class T, class = void>
class IteratorTraits {};

template<class T>
class IteratorTraits<T, void_t<typename T::IteratorCategory>> {
public:
  using IteratorCategory = typename T::IteratorCategory;
  using ValueType = typename T::ValueType;
  using Pointer = typename T::Pointer;
  using Reference = typename T::Reference;
  using DifferenceType = typename T::DifferenceType;
};

template<class T>
class IteratorTraits<T*, void> {
public:
  using IteratorCategory = RandomAccessIteratorTag;
  using ValueType = T;
  using Pointer = T*;
  using Reference = T&;
  using DifferenceType = ptrdiff_t;
};

template<class T>
class IteratorTraits<const T*, void> {
public:
  using IteratorCategory = RandomAccessIteratorTag;
  using ValueType = T;
  using Pointer = const T*;
  using Reference = const T&;
  using DifferenceType = ptrdiff_t;
};

template <class T>
class IsIterator {
private:
  template<class U>
  static FalseType test(...);
  template<class U>
  static auto test(int) -> decltype(typename IteratorTraits<U>::IteratorCategory(), TrueType());
public:
  static const bool value = decltype(test<T>(0))::value;
};

template<class Iterator>
inline typename IteratorTraits<Iterator>::IteratorCategory
IteratorCategory(const Iterator&) {
  typedef typename IteratorTraits<Iterator>::IteratorCategory Category;
  return Category();
}

template<class Iterator>
inline typename IteratorTraits<Iterator>::DifferenceType *
DistanceType(const Iterator&) { 
  return static_cast<typename IteratorTraits<Iterator>::DifferenceType *>(0); 
}

template<class Iterator>
inline typename IteratorTraits<Iterator>::ValueType *
ValueType(const Iterator&) { 
  return static_cast<typename IteratorTraits<Iterator>::ValueType *>(0); 
}

template<class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType 
__Distance(InputIterator first, InputIterator last, InputIteratorTag) {
  typename IteratorTraits<InputIterator>::DifferenceType n = 0;
  while(first!=last) {
    ++first;
    ++n;
  } 
  return n;
}

template<class RandomAccessIterator>
inline typename IteratorTraits<RandomAccessIterator>::DifferenceType 
__Distance(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIteratorTag) { 
  return last - first; 
}

template<class InputIterator>
inline typename IteratorTraits<InputIterator>::DifferenceType 
Distance(InputIterator first, InputIterator last) {
  return __Distance(first, last, IteratorCategory(first));
}

template<class InputIterator, class Distance>
inline void __Advance(InputIterator &i, Distance n, InputIteratorTag) { 
  while (n--) { ++i; } 
}

template<class BidirectionalIterator, class Distance>
inline void __Advance(BidirectionalIterator &i, Distance n, BidirectionalIteratorTag) {
  if(n > 0) {
    while(n--) { ++i; }
  }
  else {
    while(n++) { --i; }
  }
}

template<class RandomAccessIterator, class Distance>
inline void __Advance(RandomAccessIterator &i, Distance n, RandomAccessIteratorTag) {
  i += n;
}

template<class InputIterator, class Distance>
inline void Advance(InputIterator &i, Distance n) {
  __Advance(i, n, IteratorCategory(i)); 
}

} // namespace easystl

#endif // EASYSTL_ITERATOR_H_