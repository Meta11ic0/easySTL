#pragma once

namespace easystl 
{

template <typename T>
inline const T& max(const T& a, const T& b) 
{
    return a > b ? a : b;
}

template <typename InputIterator, typename OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result) 
{
    while (first != last) 
    {
        *result = *first;
        ++result;
        ++first;
    }
    return result;
}

template <typename BidirectionalIterator1, typename BidirectionalIterator2>
BidirectionalIterator2 copybackward(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result) 
{
    while (first != last)
    {
        *(--result) = *(--last);
    }
    return result;
}

template <typename ForwardIterator, typename T>
void fill(ForwardIterator first, ForwardIterator last, const T& value) 
{
    while (first != last) 
    {
        *first = value;
        ++first;
    }
}

template <typename T>
void swap(T& a, T& b) 
{
    T temp = a;
    a = b;
    b = temp;
}

} // namespace easystl
