#ifndef MDK_POOL_H_INCLUDED
#define MDK_POOL_H_INCLUDED

#include "mdk_Allocator.h"
#include "mdk_Threading.h"

namespace mdk
{

template<typename T>
struct ObjectPoolTraitsDefault
{
    typedef T ObjectType;
    typedef SyncWithAtomic SyncType;
};

class ObjectPoolNode
{
    m_noncopyable (ObjectPoolNode)

public:
    Allocator& allocator_;
    void* _memory;
    size_t _capacity;
    ObjectPoolNode* _nextNode;

    ObjectPoolNode (Allocator& allocator, size_t itemSize, size_t capacity);
    ~ObjectPoolNode();

    Allocator& getAllocator() { return allocator_; }
};

/*! An object pool based on
    http://www.codeproject.com/Articles/746630/O-Object-Pool-in-Cplusplus v-2014-04-21
*/
template<typename TRAITS>
class ObjectPool
{
    m_noncopyable (ObjectPool)

public:
    typedef typename TRAITS::ObjectType Object;
    typedef typename TRAITS::SyncType Sync;
    typedef ScopedSyncRead<Sync> SyncRead;
    typedef ScopedSyncWrite<Sync> SyncWrite;

    explicit ObjectPool (size_t initialCapacity = 32, size_t nodeMaxCapacity = 1000000, Allocator& allocator = CrtAllocator::get());

    virtual ~ObjectPool();

    Allocator& getAllocator()
    {
        return _allocator;
    }

    bool isOwnerOf (Object* content) const;

    Object* allocate();

    void release (Object* content);

    bool releaseSafe (Object* content);

private:
    enum { cItemSize = ((sizeof (Object) + sizeof (void*)-1) / sizeof (void*)) * sizeof (void*) };

    Sync _syncHandle;
    Allocator& _allocator;

    void* _nodeMemory;
    Object* _firstDeleted;
    size_t _countInNode;
    size_t _nodeCapacity;
    ObjectPoolNode _firstNode;
    ObjectPoolNode* _lastNode;
    size_t _nodeMaxCapacity;


    void _allocateNewNode();

};

template<typename TRAITS>
struct TypeUseAllocator< ObjectPool<TRAITS> >
{
    static const bool Value = true;
};

template<>
struct TypeUseAllocator<ObjectPoolNode>
{
    static const bool Value = true;
};

} // namespace

#define m_new_with_pool(pool, type) new (pool.allocate())type

template<typename POOL, typename TYPE>
void m_del_with_pool (POOL& pool, TYPE* ptr)
{
    if (nullptr == ptr)
        return;

    ptr->~TYPE();
    pool.release (ptr);
}

#include "mdk_Pool.inl"

#endif // MDK_POOL_H_INCLUDED
