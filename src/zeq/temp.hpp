
#ifndef _ZEQ_TEMP_HPP_
#define _ZEQ_TEMP_HPP_

#include "define.hpp"
#include <vector>

#ifndef ZEQ_WINDOWS
#include <sys/mman.h>
#endif

class Temp
{
private:
    struct TempArea;

    struct TempHeader
    {
        union
        {
            struct
            {
                TempArea* prevArea;
                uint32_t capacity;
            };
            uint32_t align[4]; // Memory should be allocated on 16-byte boundaries
        };
    };
    
    struct TempArea
    {
        TempHeader  header;
        byte        dataArea[16]; // Stub
    };
    
    struct TempMem
    {
        union
        {
            uint32_t byteSize;
            uint32_t align[4];
        };
        byte dataArea[16]; // Stub
    };
    
    uint32_t    m_capacity;
    byte*       m_bumpPtr;
    TempArea*   m_memoryArea;
    TempArea*   m_prevAreas;
    
    static const uint32_t DEFAULT_MEMORY_BYTES = (1 << 24); // 2^24 = 16 MB
    
private:
    size_t              bumpSpace();
    void                newArea(size_t bytes);
    static TempArea*    areaAlloc(size_t bytes);
    static void         areaFree(TempArea* area);

public:
    Temp();
    ~Temp();

    void* alloc(size_t bytes);
    void* realloc(size_t bytes, const void* ptr);

    void reset();
};
/*
static Temp gTemp;

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
*/
#endif//_ZEQ_TEMP_HPP_
