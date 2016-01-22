
#include "temp.hpp"

extern Temp gTemp;

Temp::Temp()
    : m_capacity(DEFAULT_MEMORY_BYTES),
      m_bumpPtr(nullptr),
      m_memoryArea(nullptr),
      m_prevAreas(nullptr)
{
    m_memoryArea = areaAlloc(DEFAULT_MEMORY_BYTES);
    
    if (!m_memoryArea)
    {
        //throw
    }
    
    m_bumpPtr = m_memoryArea->dataArea;
}

Temp::~Temp()
{
    reset();
    
    if (m_memoryArea)
        areaFree(m_memoryArea);
}

Temp::TempArea* Temp::areaAlloc(size_t bytes)
{
    TempArea* area;
#ifdef ZEQ_WINDOWS
    area = (TempArea*)VirtualAlloc(nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    area = (TempArea*)mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#endif
    area->header.prevArea = nullptr;
    area->header.capacity = bytes;
    
    return area;
}

void Temp::areaFree(TempArea* area)
{
#ifdef ZEQ_WINDOWS
    VirtualFree(area, 0, MEM_RELEASE);
#else
    munmap(area, area->header.capacity);
#endif
}

void Temp::reset()
{
    TempArea* prev  = m_prevAreas;
    m_bumpPtr       = m_memoryArea->dataArea;
    m_prevAreas     = nullptr;
    
    while (prev)
    {
        TempArea* next = prev->header.prevArea;
        areaFree(prev);
        prev = next;
    }
}

void Temp::newArea(size_t bytes)
{
    size_t cap = m_capacity;
    
    while (bytes > cap)
        cap *= 2;
    
    TempArea* area = areaAlloc(cap);
    
    m_capacity  = cap;
    m_bumpPtr   = area->dataArea;
    
    TempArea* old           = m_memoryArea;
    m_memoryArea            = area;
    old->header.prevArea    = m_prevAreas;
    m_prevAreas             = old;
}

size_t Temp::bumpSpace()
{
    return (uintptr_t)((((byte*)m_memoryArea) + m_capacity) - m_bumpPtr);
}

#define alignedbytes(bytes) (((bytes) + 16 - 1) & (~(16 - 1)))

void* Temp::alloc(size_t bytes)
{
    bytes = alignedbytes(bytes) + 16;
    
    if (bumpSpace() < bytes)
        newArea(bytes);
    
    TempMem* ret = (TempMem*)m_bumpPtr;
    m_bumpPtr += bytes;
    ret->byteSize = bytes;
    return ret->dataArea;
}

void* Temp::realloc(size_t bytes, const void* ptr)
{
    if (ptr)
    {
        TempMem* mem = (TempMem*)(((byte*)ptr) - 16);
        
        uint32_t b          = alignedbytes(bytes) + 16;
        uint32_t curSize    = mem->byteSize;
        uint32_t diff;
        byte* end;
        
        // Can we just extend the existing area?
        if (curSize >= b)
            goto ret_ptr;
        
        end     = ((byte*)mem) + curSize;
        diff    = b - curSize;
        
        if (end == m_bumpPtr && bumpSpace() >= diff)
        {
            m_bumpPtr += diff;
            mem->byteSize = b;
        ret_ptr:
            return (void*)ptr;
        }
    }
    
    return alloc(bytes);
}
