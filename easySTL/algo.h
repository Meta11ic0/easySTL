#ifndef EASYSTL_ALGO_H_
#define EASYSTL_ALGO_H_

namespace easystl {

template <typename T>
inline const T& Max(const T& a, const T& b) noexcept { return a > b ? a : b; }

template <typename InputIterator, typename OutputIterator>
OutputIterator Copy(InputIterator first, InputIterator last, OutputIterator result) noexcept {
  while (first != last) {
    *result = *first;
    ++result;
    ++first;
  }
  return result;
}

template <typename BidirectionalIterator1, typename BidirectionalIterator2>
BidirectionalIterator2 Copybackward(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result) noexcept {
  while (first != last) { 
    *(--result) = *(--last); 
  }
  return result;
}

template <typename ForwardIterator, typename T>
void Fill(ForwardIterator first, ForwardIterator last, const T& value) noexcept {
  while (first != last) {
    *first = value;
    ++first;
  }
}

template <typename T>
void Swap(T& a, T& b) noexcept {
  T temp = a;
  a = b;
  b = temp;
}

} // namespace easystl

#endif // EASYSTL_ALGO_H_