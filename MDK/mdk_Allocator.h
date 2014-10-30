#ifndef MDK_ALLOCATOR_H_INCLUDED
#define MDK_ALLOCATOR_H_INCLUDED

#include "mdk_Config.h"
#include <new>
#include <cstdlib>

namespace mdk
{

class Allocator
{
public:
    //! Return nullptr if the requested size cannot be allocated
    virtual void* malloc (size_t size) = 0;

    //! when origPtr == nullptr, it should behave as Allocator::malloc()
    virtual void* realloc (void* origPtr, size_t origSize, size_t newSize) = 0;

    //! should be able to handle ptr == nullptr
    virtual void free (void* ptr) = 0;
};

class CrtAllocator : public Allocator
{
public:
    void* malloc (size_t size) override
    {
        return std::malloc (size);
    }

    void* realloc (void* origPtr, size_t origSize, size_t newSize) override
    {
        (void) origSize;
        return std::realloc (origPtr, newSize);
    }

    void free (void* ptr) override
    {
        std::free (ptr);
    }

    static CrtAllocator& get();
};

template<typename Type>
struct UseAllocator
{
    static const bool Value = false;
};

template<typename Type, bool AllocatorUsage>
struct ConstructWithAllocator
{
    template<typename... Args>
    static void invoke (Allocator& alloc, Type* ptr, Args... args)
    {
        new (ptr) Type (args...);
    }
};

template<typename Type>
struct ConstructWithAllocator<Type, true>
{
    template<typename... Args>
    static void invoke (Allocator& alloc, Type* ptr, Args... args)
    {
        new (ptr) Type (args..., alloc);
    }
};


}   // namespace

#define m_new(alloc, type) new (alloc.malloc (sizeof (type))) type

template<typename TYPE>
void m_del (mdk::Allocator& alloc, TYPE* ptr)
{
    if (nullptr == ptr)
        return;

    ptr->~TYPE();
    alloc.free (ptr);
}


#endif // MDK_ALLOCATOR_H_INCLUDED
