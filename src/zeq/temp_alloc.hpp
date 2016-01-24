
#ifndef _ZEQ_TEMP_ALLOC_HPP_
#define _ZEQ_TEMP_ALLOC_HPP_

#include "temp.hpp"

extern Temp gTemp;

template <typename T>
class TempAllocator
{
public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef T*          pointer;
    typedef const T*    const_pointer;
    typedef T&          reference;
    typedef const T&    const_reference;
    typedef T           value_type;

    template <typename X>
    struct rebind
    {
        typedef TempAllocator<X> other;
    };
    
    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }
    
    pointer allocate(size_type n, const void* hint = nullptr)
    {
        return (pointer)gTemp.realloc(n * sizeof(value_type), hint);
    }
    
    void deallocate(pointer p, size_type n)
    {
        (void)p;
        (void)n;
    }
    
    size_type max_size() const throw()
    {
        return size_t(-1) / sizeof(T);
    }
};

template <typename T>
class TempVector : public std::vector<T, TempAllocator<T>> { };

#endif//_ZEQ_TEMP_ALLOC_HPP_

