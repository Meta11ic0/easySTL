#ifndef EASYSTL_CONSTRUCTOR_H_
#define EASYSTL_CONSTRUCTOR_H_

#include <new>

namespace easystl {

template<class T> 
inline void Construct(T* p) { new(p) T(); }

template<class T1, class T2>
inline void Construct(T1* p, const T2 & value) { new(p) T1(value); }

template<class T>
inline void Destroy(T* p) { p->~T(); }

template<class ForwardIterator>
inline void Destroy(ForwardIterator first, ForwardIterator last) {
  for(; first != last; ++first) {
    Destroy(&*first);
  }
}

} // namespace easystl

#endif // EASYSTL_CONSTRUCTOR_H_