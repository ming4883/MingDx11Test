#ifndef MDK_POOL_H_INCLUDED
#define MDK_POOL_H_INCLUDED

#include "mdk_Allocator.h"
#include "mdk_SyncPrimitive.h"

namespace mdk
{

template<typename T>
struct ObjectPoolTraitsDefault
{
    typedef T ObjectType;
    typedef CrtAllocator AllocatorType;
    typedef SyncWithAtomic SyncType;
};

/*! An object pool based on
    http://www.codeproject.com/Articles/746630/O-Object-Pool-in-Cplusplus v-2014-04-21
*/
template<typename TRAITS>
class ObjectPool
{
public:
    typedef typename TRAITS::ObjectType Object;
    typedef typename TRAITS::AllocatorType Allocator;
    typedef typename TRAITS::SyncType Sync;
    typedef ScopedSyncRead<Sync> SyncRead;
    typedef ScopedSyncWrite<Sync> SyncWrite;
    
    explicit ObjectPool (size_t initialCapacity = 32, size_t nodeMaxCapacity = 1000000);

    virtual ~ObjectPool();

    bool isOwnerOf (Object* content) const;

    Object* allocate();

    void release (Object* content);

    bool releaseSafe (Object* content);

private:
    enum { ConstItemSize = ((sizeof (Object) + sizeof (void*)-1) / sizeof (void*)) * sizeof (void*) };

    class _Node
    {
    public:
        Allocator& _allocator;
        void* _memory;
        size_t _capacity;
        _Node* _nextNode;

        _Node (Allocator& allocator, size_t capacity);

        ~_Node();

    private:
        _Node (const _Node& source);
        void operator = (const _Node& source);
    };

    Sync _syncHandle;
    Allocator _allocator;

    void* _nodeMemory;
    Object* _firstDeleted;
    size_t _countInNode;
    size_t _nodeCapacity;
    _Node _firstNode;
    _Node* _lastNode;
    size_t _nodeMaxCapacity;
    

    void _allocateNewNode();

    // non copyable
    ObjectPool (const ObjectPool& source);
    void operator = (const ObjectPool& source);

};

}   // namespace

#include "mdk_Pool.inl"

#endif	// MDK_POOL_H_INCLUDED
