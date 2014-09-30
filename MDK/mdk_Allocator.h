#ifndef MDK_ALLOCATOR_H_INCLUDED
#define MDK_ALLOCATOR_H_INCLUDED

#include <new>

namespace mdk
{

class CrtAllocator
{
public:
    void* malloc (size_t size)
	{
		return std::malloc (size);
	}
	
	void* realloc (void* origPtr, size_t origSize, size_t newSize)
	{
		(void) origSize;
		return std::realloc (origPtr, newSize);
	}
	
	void free (void* ptr)
	{
		std::free (ptr);
	}
};

}   // namespace

#define m_new(alloc, type) new (alloc.malloc (sizeof (type))) type

template<typename A, typename T>
void m_del (A& alloc, T* ptr)
{
    ptr->~T();
    alloc.free (ptr);
}


#endif	// MDK_ALLOCATOR_H_INCLUDED
