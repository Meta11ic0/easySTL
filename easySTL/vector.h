#ifndef EASYSTL_VECTOR_H_
#define EASYSTL_VECTOR_H_

#include "allocator.h"
#include "constructor.h"
#include "iterator.h"
#include "uninitialized.h"
#include "algo.h"

namespace easystl {

template<class T, class Alloc = Allo>
class vector {
 public:
  // type alias
  using value_type      = T;
  using pointer         = T*;
  using iterator        = T*;
  using reference       = T&;
  using size_type       = size_t;
  using difference_type = ptrdiff_t;
  // constructor
  vector() noexcept : begin_(nullptr), end_(nullptr), capacity_(nullptr) {}
  vector(size_type len, const T& value) noexcept { NumsInit(len, value); }
  explicit vector(size_type len) noexcept { NumsInit(len, T()); }
  template<class Iterator, typename std::enable_if_t<IsIterator<Iterator>::value, int> = 0>
  vector(Iterator first, Iterator last) noexcept {
    RangeInit(first, last);
  }

  vector(std::initializer_list<value_type> ilist) {
    RangeInit(ilist.begin(), ilist.end());
  }
  // copy constructor
  vector(const vector& other) noexcept {
    RangeInit(other.begin_, other.end_);
  }
  // copy assignment operator
  vector& operator=(const vector& rhs) noexcept {
    if(this != &rhs) {
      const size_type rhslen = rhs.size();
      if(rhslen > capacity()) {
        vector tmp(rhs);
        swap(tmp);
      }
      else if(size() >= rhslen) {
        auto i = Copy(rhs.begin_, rhs.end_, begin_);
        Destroy(i, end_);
        end_ = begin_ + rhslen;
      }
      else {
        Copy(rhs.begin_, rhs.end_, begin_);
        uninitialized_copy(rhs.begin_ + size(), rhs.end_, end_);
        end_ = begin_ + rhslen;
      }  
    }
    return *this;
  }
  vector& operator=(std::initializer_list<value_type> ilist) noexcept {
    vector<T> tmp(ilist);
    swap(tmp);
    return *this;
  }
  // destructor
  ~vector() noexcept { DestroynDeallocate(begin_, end_, static_cast<size_type>(capacity_ - begin_)); }
  // basic operation
  iterator begin() noexcept { return begin_; }
  iterator end() noexcept { return end_; }
  size_type size() const noexcept { return static_cast<size_type>(end_ - begin_); }
  bool empty() const noexcept { return begin_ == end_; }
  size_type capacity() const noexcept { return static_cast<size_type>(capacity_ - begin_); }
  reference operator[] (size_type n) noexcept { return *(begin_ + n); }
  reference front() noexcept { return *begin(); }
  reference back()noexcept { return *(end_ - 1); }
  void push_back(const T& x) noexcept {
    if(end_ != capacity_) {
      Construct(end_, x);
      ++end_;
    }
    else {
      InsertAux(end_, 1, x);
    }
  }
  void pop_back() noexcept {
    if(empty()) { return; }
    --end_;
    Destroy(end_);
  }
  iterator erase(iterator pos) noexcept {
    Copy(pos + 1, end_, pos); 
    Destroy(end_);
    --end_;
    return pos;
  }
  iterator erase(iterator first, iterator last) noexcept {
    if(first!=last) {
      auto i = Copy(last, end_, first);
      Destroy(i, end_);
      end_ = end_ - static_cast<size_type>(last - first);
    }
    return last;
  }
  void resize(size_type newsize, const T& x) noexcept {
    if(newsize < size()) {
      erase(begin_ + newsize, end_);
    }
    else {
      insert(end_, newsize - size(), x);
    }
  }
  void resize(size_type newsize) noexcept { resize(newsize, T()); }
  void clear() noexcept { erase(begin_, end_); }
  pointer data() noexcept { return begin_; }
  // swap vector
  void swap(vector& rhs) noexcept {
    if(this != &rhs) {
      easystl::Swap(begin_, rhs.begin_);
      easystl::Swap(end_, rhs.end_);
      easystl::Swap(capacity_, rhs.capacity_);
    }
  }
  // insert
  iterator insert(iterator pos, size_type size, const T& x) noexcept {
    // leftbytes enough
    if(size_type(capacity_ -  end_) >= size) {
      const size_type elemsafter = end_ - pos;
      iterator oldend = end_;
      if(elemsafter > size) {
        uninitialized_copy(end_ - size, end_, end_);
        end_ += size;
        Copybackward(pos, oldend - size, oldend);
        Fill(pos, pos + size, x);
      }
      else {
        uninitialized_fill_n(end_, size - elemsafter, x);
        end_ += size - elemsafter;
        uninitialized_copy(pos, oldend, end_);
        end_ += elemsafter;
        Fill(pos, oldend, x);
      }
    }
    // leftbytes not enough
    else {
      InsertAux(pos, size, x);
    }
    return pos;
  }
  
  template<class Iter1, class Iter2,
    typename std::enable_if_t<IsIterator<Iter1>::value, int> = 0, 
    typename std::enable_if_t<IsIterator<Iter2>::value, int> = 0>
  iterator insert(Iter1 pos, Iter2 first, Iter2 last) noexcept {
    if (first == last) return pos;
    const auto size = last - first;
    // leftbytes enough
    if (size_type(capacity_ - end_) >= size) {
      const size_type elemsafter = end_ - pos;
      iterator oldend = end_;
      if (elemsafter > size) {
        uninitialized_copy(end_ - size, end_, end_);
        end_ += size;
        Copybackward(pos, oldend - size, oldend);
        Copy(first, last, pos);
      }
      else {
        uninitialized_fill_n(end_, size - elemsafter, T());
        end_ += size - elemsafter;
        uninitialized_copy(pos, oldend, end_);
        end_ += elemsafter;
        Copy(first, last, pos);
      }
    }
    // leftbytes not enough
    else {
      InsertAux(pos, first, last);
    }
  }
  iterator insert(iterator pos, const T& x) noexcept {
    return insert(pos, 1, x);
  }
  // assign
  void assign(size_type n, const T& value) noexcept {
    clear();
    if (n > capacity()) {
      vector tmp(n, value);
      swap(tmp);
    }
    else {
      uninitialized_fill_n(begin_, n, value);
      end_ = begin_ + n;
    }
  }
  template<class Iter, typename std::enable_if_t<IsIterator<Iter>::value, int> = 0>
  void assign(Iter first, Iter last) noexcept {
    clear();
    const size_type len = last - first;
    if (len > capacity()) {
      vector tmp(first, last);
      swap(tmp);
    }
    else {
      uninitialized_copy(first, last, begin_);
      end_ = begin_ + len;
    }
  }
  void assign(std::initializer_list<value_type> ilist) noexcept {
    assign(ilist.begin(), ilist.end());
  }
 private:
  // allocator
  using DataAllocator = AllocatorWrapper<T, Alloc>;
  // allocate memory and construct 
  // memory size is n * sizeof(T)
  // construct n variables of type T with values of value
  // initialize
  void NumsInit(size_type n, const T& value) noexcept {
    size_type initsize = easystl::Max(static_cast<size_type>(n), static_cast<size_type>(16));
    iterator current = DataAllocator::Allocate(initsize);
    begin_ = current;
    end_  = easystl::uninitialized_fill_n(current, n, value);
    capacity_ = begin_ + initsize;
  }
  // range init
  template <class Iter>
  void RangeInit(Iter first, Iter last) noexcept {
    size_type initsize = easystl::Max(static_cast<size_type>(last - first), static_cast<size_type>(16));
    begin_ =  DataAllocator::Allocate(initsize);
    end_ = easystl::uninitialized_copy(first, last, begin_);
    capacity_ = begin_ + initsize;
  }
  // destroy and Deallocate
  void DestroynDeallocate(iterator first, iterator last, size_type len) noexcept {
    if(first) {
      Destroy(first, last);
      DataAllocator::Deallocate(first, len);
    }
  }
 // inesert when space not enough
  void InsertAux(iterator pos, size_type nums, const T& x) noexcept {
    const size_type newsize = Max(size()*2, static_cast<size_type>(16));
    iterator newbegin = DataAllocator::Allocate(newsize);
    iterator newend = newbegin;
    newend = uninitialized_copy(begin_, pos, newbegin);
    newend = uninitialized_fill_n(newend, nums, x);
    newend = uninitialized_copy(pos, end_, newend);
    DestroynDeallocate(begin_, end_, static_cast<size_type>(capacity_ - begin_));
    begin_ = newbegin;
    end_ = newend;
    capacity_ = begin_ + newsize;
  }
  template<class Iter1, class Iter2,
    typename std::enable_if_t<IsIterator<Iter1>::value, int> = 0,
    typename std::enable_if_t<IsIterator<Iter2>::value, int> = 0>
  void InsertAux(Iter1 pos, Iter2 first, Iter2 last) noexcept {
    {
      const size_type newsize = Max(size() * 2, static_cast<size_type>(16));
      iterator newbegin = DataAllocator::Allocate(newsize);
      iterator newend = newbegin;
      newend = uninitialized_copy(begin_, pos, newbegin);
      newend = uninitialized_copy(first, last, newend);
      newend = uninitialized_copy(pos, end_, newend);
      DestroynDeallocate(begin_, end_, static_cast<size_type>(capacity_ - begin_));
      begin_ = newbegin;
      end_ = newend;
      capacity_ = begin_ + newsize;
    }
  }

  // flag
  iterator begin_;    // flag for used memory head 
  iterator end_;      // flag for used memory tail 
  iterator capacity_; // flag for available memory tail 
};

} // namespace easystl

#endif // EASYSTL_VECTOR_H_
