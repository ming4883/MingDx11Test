#ifndef MDK_SOAMANAGER_H_INCLUDED
#define MDK_SOAMANAGER_H_INCLUDED

#include "mdk_Config.h"
#include "mdk_Allocator.h"
#include "mdk_Threading.h"

#include <tuple>
#include <type_traits>

namespace mdk
{

//! template for Struct-Of-Arrays
template<typename... DATA>
struct SOAManagerTraitsDefault
{
    typedef std::tuple<typename std::add_pointer<DATA>::type ...> SOAType;
    typedef SyncWithAtomic SyncType;
    typedef juce::uint32 HandleDataType;

    enum
    {
        cHandleIndexBits = 20,
        cHandleGenerationBits = 12,
    };

};

template<typename MANAGER, size_t INDEX>
struct SOAColumn
{
    enum {cIndex = INDEX};
    typedef typename MANAGER::SOA SOA;
    typedef typename std::tuple_element<INDEX, SOA>::type ElemPtr;
    typedef typename std::remove_pointer<ElemPtr>::type Elem;
};

/*! A template for managing objects in data-oriented manner.
    For performance reasons, the constructor and destructor will NOT be invoked during acquire() and release();
    and OBJECT must support operator =.
*/
template<typename TRAITS>
class SOAManager
{
    m_noncopyable (SOAManager)

public:
    typedef typename TRAITS::SOAType SOA;
    typedef typename TRAITS::SyncType Sync;
    typedef ScopedSyncRead<Sync> SyncRead;
    typedef ScopedSyncWrite<Sync> SyncWrite;

    enum { cSOASize =  std::tuple_size<SOA>::value };
    
    struct Handle;

public:
    //! Construct the SOAManager with initial capacity.
    SOAManager (size_t initialCapacity, Allocator& allocator = CrtAllocator::get());

    virtual ~SOAManager();

    Allocator& getAllocator() { return _allocator; }

    //! Returns the capacity of this SOAManager.
    size_t capacity() const
    {
        return mCapacity;
    }

    //! Returns the number of objects being allocated in this SOAManager.
    size_t size() const
    {
        return mSize;
    }

    //! Returns the number of enabled-objects being allocated in this SOAManager.
    size_t sizeOfEnabled() const
    {
        return mFirstDisabled;
    }

    //! Returns the number of disabled-objects being allocated in this SOAManager.
    size_t sizeOfDisabled() const
    {
        return mSize - mFirstDisabled;
    }

    //! Retuns a handle with points to a memory location. If this SOAManager is full, an invalid handle is returned.
    Handle acquire();

    //! Retuns true of the memory location
    bool release (Handle handle);

    //! An utility which invoke the object's constructor.
    template<typename SOACOL, typename... ARGS>
    bool construct (Handle handle, ARGS... args)
    {
        typedef typename SOACOL::Elem Elem;
        SyncWrite sync (mSyncHandle);

        if (!isValidNoSync (handle))
            return false;

        ConstructWithAllocator<Elem, UseAllocator<Elem>::Value>::invoke (_allocator, getNoSync<SOACOL> (handle), args...);

        return true;
    }

    template<typename SOACOL>
    bool destruct (Handle handle)
    {
        typedef typename SOACOL::Elem Elem;

        SyncWrite sync (mSyncHandle);

        if (!isValidNoSync (handle))
            return false;

        getNoSync<SOACOL> (handle)->~Elem();
        return true;
    }

    //! Returns to if handle is pointing to a memory location within this SOAManager, otherwise false.
    bool isValid (Handle handle) const;
    
    /*! Get the memory location associated with the given handle.
        Since the memory locations may be changed during resize() or release(), do NOT store the returned pointer for later usages.
     */
    template<typename SOACOL>
    typename SOACOL::ElemPtr get (Handle handle);

    /*! Fetch the content of the memory location with synchronization primitive support.
     */
    template<typename SOACOL>
    bool fetch (typename SOACOL::Elem& data, Handle handle) const;

    /*! Store the content to the memory location with synchronization primitive support.
     */
    template<typename SOACOL>
    bool store (Handle handle, const typename SOACOL::Elem& data);

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
    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr* beginOfAll() const
    {
        return std::get<SOACOL::cIndex> (mSOAs);
        //return mDataSlot;
    }

    /*! Returns a pointer to the element which follows the last element of all objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr* endOfAll() const
    {
        return std::get<SOACOL::cIndex> (mSOAs) + mSize;
        //return mDataSlot + mSize;
    }

    /*! Returns a pointer to the first element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr beginOfEnabled() const
    {
        return std::get<SOACOL::cIndex> (mSOAs);
        //return mDataSlot;
    }

    /*! Returns a pointer to the element which follows the last element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr endOfEnabled() const
    {
        return std::get<SOACOL::cIndex> (mSOAs) + mFirstDisabled;
        //return mDataSlot + mFirstDisabled;
    }

    /*! Returns a pointer to the first element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr beginOfDisabled() const
    {
        return std::get<SOACOL::cIndex> (mSOAs) + mFirstDisabled;
        //return mDataSlot + mFirstDisabled;
    }

    /*! Returns a pointer to the element which follows the last element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr endOfDisabled() const
    {
        return std::get<SOACOL::cIndex> (mSOAs) + mSize;
        //return mDataSlot + mSize;
    }

protected:
    struct SOAops;

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
    SOA mSOAs;              //! struct-of-arrays for holding the actual data
    Handle* mDataHandle;    //! mapping data slot back to the associated Handle, used in copyDataSlot() & swapDataSlot().

    //! Returns true if successfully resized, otherwise false (e.g. out of memory or newCapacity > Handle::kIndexLimit).
    bool resize (size_t newCapacity);

    void swapDataSlot (size_t slotIdxA, size_t slotIdxB);

    m_inline bool isEnabledNoSync (Handle handle) const;

    m_inline bool isValidNoSync (Handle handle) const;

    template<typename SOACOL>
    m_inline typename SOACOL::ElemPtr getNoSync (Handle handle) const;

    m_inline Handle acquireNoSync();

    m_inline bool releaseNoSync (Handle handle);
};

}

#include "mdk_SOAManager.inl"

#endif  // MDK_SOAMANAGER_H_INCLUDED
