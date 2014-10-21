#ifndef MDK_MANAGER_H_INCLUDED
#define MDK_MANAGER_H_INCLUDED

#include "mdk_Allocator.h"
#include "mdk_Threading.h"

namespace mdk
{

template<typename T>
struct ManagerTraitsDefault
{
    typedef T DataType;
    typedef SyncWithAtomic SyncType;
    typedef juce::uint32 HandleDataType;

    enum
    {
        kHandleIndexBits = 20,
        kHandleGenerationBits = 12,
    };
    
};

/*! A template for managing objects in data-oriented manner.
    For performance reasons, the constructor and destructor will NOT be invoked during acquire() and release();
    and OBJECT must support operator =.
*/
template<typename TRAITS>
class Manager
{
    m_noncopyable (Manager)

public:
    struct Handle
    {
        typedef typename TRAITS::HandleDataType DataType;
        DataType index : TRAITS::kHandleIndexBits;
        DataType generation : TRAITS::kHandleGenerationBits;

        enum
        {
            kIndexLimit = 1 << (TRAITS::kHandleIndexBits),
            kGenerationLimit = 1 << (TRAITS::kHandleGenerationBits),
        };

        Handle()
        {
        }

        Handle (DataType rawbits)
        {
            m_static_assert (sizeof (Handle) == sizeof (DataType));
            *this = *reinterpret_cast<Handle*> (&rawbits);
        }

        operator DataType() const
        {
            m_static_assert (sizeof (Handle) == sizeof (DataType));
            return *reinterpret_cast<DataType*> (const_cast<Handle*> (this));
        }
    };
    typedef typename TRAITS::DataType Data;
    typedef typename TRAITS::SyncType Sync;
    typedef ScopedSyncRead<Sync> SyncRead;
    typedef ScopedSyncWrite<Sync> SyncWrite;

protected:

    enum
    {
        kInvalidSlotIndex = 1 << (TRAITS::kHandleIndexBits)
    };

    //! Allocation Table
    struct ATable
    {
        Handle handle;
        size_t slotIndex;
        size_t freelistNext;
    };

    Allocator& _allocator;
    Sync mSyncHandle;
    size_t mCapacity;
    size_t mSize;
    size_t mFirstDisabled;
    size_t mFreelistDequeue;
    size_t mFreelistEnqueue;
    size_t mFreelistCount;
    ATable* mATable;
    Data* mDataSlot;      //! slots for holding the actual data
    Handle* mDataHandle;    //! mapping data slot back to the associated Handle, used in copyDataSlot() & swapDataSlot().
    
    //! Returns true if successfully resized, otherwise false (e.g. out of memory or newCapacity > Handle::kIndexLimit).
    bool resize (size_t newCapacity);

    void copyDataSlot (size_t from, size_t to);

    void swapDataSlot (size_t a, size_t b);

    inline bool isEnabledNoSync (Handle handle) const;

    inline bool isValidNoSync (Handle handle) const;

    inline Data* getNoSync (Handle handle) const;

    inline Handle acquireNoSync();

    inline bool releaseNoSync (Handle handle);


public:
    //! Construct the Manager with initial capacity.
    Manager (size_t initialCapacity, Allocator& allocator = CrtAllocator::get());

    virtual ~Manager();

    //! Returns the capacity of this Manager.
    size_t capacity() const
    {
        return mCapacity;
    }

    //! Returns the number of objects being allocated in this Manager.
    size_t size() const
    {
        return mSize;
    }

    //! Retuns a handle with points to a memory location. If this Manager is full, an invalid handle is returned.
    Handle acquire();

    //! Retuns true of the memory location
    bool release (Handle handle);

    //! Returns to if handle is pointing to a memory location within this Manager, otherwise false.
    bool isValid (Handle handle) const;

    /*! Get the memory location associated with the given handle.
        Since the memory locations may be changed during resize() or release(), do NOT store the returned pointer for later usages.
     */
    Data* get (Handle handle);

    /*! Fetch the content of the memory location with synchronization primitive support.
     */
    bool fetch (Data& data, Handle handle) const;

    /*! Store the content to the memory location with synchronization primitive support.
     */
    bool store (Handle handle, const Data& data);

    /*! Mark the object as enabled.
     */
    void enable (Handle handle);

    /*! Mark the object as disabled.
     */
    void disable (Handle handle);

    /*! Returns true if the associated object is marked as enabled, otherwise false.
     */
    bool isEnabled (Handle handle) const;

    /*! Returns a pointer to the first element of all objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline Data* beginOfAll() const
    {
        return mDataSlot;
    }

    /*! Returns a pointer to the element which follows the last element of all objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline Data* endOfAll() const
    {
        return mDataSlot + mSize;
    }

    /*! Returns a pointer to the first element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline Data* beginOfEnabled() const
    {
        return mDataSlot;
    }

    /*! Returns a pointer to the element which follows the last element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline Data* endOfEnabled() const
    {
        return mDataSlot + mFirstDisabled;
    }

    /*! Returns a pointer to the first element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline Data* beginOfDisabled() const
    {
        return mDataSlot + mFirstDisabled;
    }

    /*! Returns a pointer to the element which follows the last element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    inline Data* endOfDisabled() const
    {
        return mDataSlot + mSize;
    }
};

//! An utility which invoke the object's default constructor just after Manager::acquire().
template<typename Manager>
inline typename Manager::Handle createObject (Manager& manager)
{
    typedef typename Manager::Data Data;
    typename Manager::Handle handle = manager.acquire();

    if (manager.isValid (handle))
        new (manager.get (handle))Data();

    return handle;
}

//! An utility which invoke the object's destructor right before Manager::release().
template<typename Manager>
inline bool destroyObject (Manager& manager, typename Manager::Handle handle)
{
    typedef typename Manager::Data Data;
    if (manager.isValid (handle))
        manager.get (handle)->~Data();

    return manager.release (handle);
}

}

#include "mdk_Manager.inl"

#endif  // MDK_MANAGER_H_INCLUDED
