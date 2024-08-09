_Pragma("once")
#include "constructor.hpp"

namespace easystl
{

template <class InputIter, class ForwardIter>
ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result)
{
    auto current = result;
    try
    {
        for(; first != last; ++first, ++current)
        {
            easystl::Construct(&*current, *first);
        }
    }
    catch(...)
    {
        easystl::Destroy(result, current);
    }
    
    return current;
}

template <class ForwardIter, class T>
void  uninitialized_fill(ForwardIter first, ForwardIter last, const T& value)
{
    auto current = first;
    try
    {
        for(; current != last; ++current)
        {
            easystl::Construct(&*current, value);
        }
    }
    catch(...)
    {
        easystl::Destroy(first, current);
    }
}

template <class ForwardIter, class Size, class T>
ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value)
{

    auto current = first;
    try
    {
        for(; n > 0; --n, ++current)
        {
            easystl::Construct(&*current, value);
        }
    }
    catch(...)
    {
        easystl::Destroy(first, current);
    }

    return current;
}

} // namespace easystl