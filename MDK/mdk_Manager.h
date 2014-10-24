#ifndef MDK_MANAGER_H_INCLUDED
#define MDK_MANAGER_H_INCLUDED

#include "mdk_Config.h"
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
        cHandleIndexBits = 20,
        cHandleGenerationBits = 12,
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
        DataType index : TRAITS::cHandleIndexBits;
        DataType generation : TRAITS::cHandleGenerationBits;

        enum
        {
            cIndexLimit = 1 << (TRAITS::cHandleIndexBits),
            cGenerationLimit = 1 << (TRAITS::cHandleGenerationBits),
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

public:
    //! Construct the Manager with initial capacity.
    Manager (size_t initialCapacity, Allocator& allocator = CrtAllocator::get());

    virtual ~Manager();

    Allocator& getAllocator() { return _allocator; }

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

    //! An utility which invoke the object's constructor.
    template<typename... Args>
    bool construct (Handle handle, Args... args)
    {
        SyncWrite sync (mSyncHandle);

        if (!isValidNoSync (handle))
            return false;

        ConstructWithAllocator<Data, UseAllocator<Data>::value>::invoke (_allocator, getNoSync (handle), args...);

        return true;
    }

    bool destruct (Handle handle)
    {
        SyncWrite sync (mSyncHandle);

        if (!isValidNoSync (handle))
            return false;

        getNoSync (handle)->~Data();
        return true;
    }

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

protected:

    enum
    {
        cInvalidSlotIndex = 1 << (TRAITS::cHandleIndexBits)
    };

    //! Allocation Table
    struct ATable
    {
        Handle handle;
        size_t slotIndex;
        size_t freelistNext;
    };

    Allocator& _allocator;
    mutable Sync mSyncHandle;
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

    void swapDataSlot (size_t slotIdxA, size_t slotIdxB);

    inline bool isEnabledNoSync (Handle handle) const;

    inline bool isValidNoSync (Handle handle) const;

    inline Data* getNoSync (Handle handle) const;

    inline Handle acquireNoSync();

    inline bool releaseNoSync (Handle handle);
};

}

#include "mdk_Manager.inl"

#endif  // MDK_MANAGER_H_INCLUDED
