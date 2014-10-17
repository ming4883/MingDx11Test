#ifndef MDK_ALLOCATOR_H_INCLUDED
#define MDK_ALLOCATOR_H_INCLUDED

#include <new>

namespace mdk
{

class Allocator
{
public:
    virtual void* malloc (size_t size) = 0;
    
    virtual void* realloc (void* origPtr, size_t origSize, size_t newSize) = 0;
    
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

}   // namespace

#define m_new(alloc, type) new (alloc.malloc (sizeof (type))) type

template<typename ALLOCATE, typename TYPE>
void m_del(ALLOCATE& alloc, TYPE* ptr)
{
    if (nullptr == ptr)
        return;

    ptr->~TYPE();
    alloc.free (ptr);
}


#endif // MDK_ALLOCATOR_H_INCLUDED
