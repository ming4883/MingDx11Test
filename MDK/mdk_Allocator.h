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

/*! To make a type support allocator, define the following:

    template<> struct TypeUseAllocator<MyType>
    {
        static const bool Value = true;
    };

    class MyType
    {
        Allocator& allocator_;
    public:
        Allocator& getAllocator() { return allocator_; }
    }
 */
template<typename TYPE>
struct TypeUseAllocator
{
    static const bool Value = false;
};

/*! Returns the number of bytes required for allocating a type.
 */
template<typename TYPE>
struct TypeSizeOf
{
    static const size_t Value = sizeof (TYPE);
};

template<typename TYPE, bool USE_ALLOCATOR>
struct TypeConstructor
{
    template<typename... Args>
    static void invoke (Allocator& /*alloc*/, TYPE* ptr, Args... args)
    {
        new (ptr) TYPE (args...);
    }
};

template<typename TYPE>
struct TypeConstructor<TYPE, true>
{
    template<typename... Args>
    static void invoke (Allocator& alloc, TYPE* ptr, Args... args)
    {
        new (ptr) TYPE (alloc, args...);
    }
};

template<typename TYPE>
struct TypeTraits
{
    static const bool UseAllocator = TypeUseAllocator<TYPE>::Value;
    static const size_t SizeOf = TypeSizeOf<TYPE>::Value;

    template<typename... Args>
    static void construct (Allocator& alloc, TYPE* ptr, Args... args)
    {
        return TypeConstructor<TYPE, UseAllocator>::invoke (alloc, ptr, args...);
    }
};

}   // namespace

//#define m_new(alloc, type) new (alloc.malloc (sizeof (type))) type
template<typename TYPE, typename... Args>
static TYPE* m_new (mdk::Allocator& alloc, Args... args)
{
    TYPE* ptr = static_cast<TYPE*> (alloc.malloc (sizeof (TYPE)));
    TypeTraits<TYPE>::construct (alloc, ptr, args...);
    return ptr;
}

template<typename TYPE>
void m_del (mdk::Allocator& alloc, TYPE* ptr)
{
    if (nullptr == ptr)
        return;

    ptr->~TYPE();
    alloc.free (ptr);
}

template<typename TYPE>
void m_del (TYPE* ptr)
{
    if (nullptr == ptr)
        return;

    m_del (ptr->getAllocator(), ptr);
}


#endif // MDK_ALLOCATOR_H_INCLUDED
