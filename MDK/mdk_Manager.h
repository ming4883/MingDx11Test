#ifndef MDK_MANAGER_H_INCLUDED
#define MDK_MANAGER_H_INCLUDED

#include "mdk_Config.h"
#include "mdk_Allocator.h"
#include "mdk_Threading.h"

#include <tuple>
#include <type_traits>

namespace mdk
{

//! template for Struct-Of-Arrays
template <typename TUPLE, size_t INDEX>
struct SOA
{
    typedef TUPLE Tuple;
    typedef typename std::tuple_element<INDEX, Tuple>::type ElemPtr;
    typedef typename std::remove_pointer<ElemPtr>::type Elem;
};

template<typename... DATA>
struct ManagerTraitsDefault
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
struct ManagerSOA : public SOA<typename MANAGER::SOAs, INDEX>
{

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

    typedef typename TRAITS::SOAType SOAs;
    typedef typename TRAITS::SyncType Sync;
    typedef ScopedSyncRead<Sync> SyncRead;
    typedef ScopedSyncWrite<Sync> SyncWrite;

    enum { cSize =  std::tuple_size<SOAs>::value };

    template<size_t INDEX>
    struct Ops : public SOA<SOAs, INDEX>
    {
        typedef Ops<INDEX+1> Next;

        static void reset (SOAs& t)
        {
            std::get<INDEX> (t) = nullptr;
            Next::reset (t);
        }

        static void malloc (SOAs& t, Allocator& allocator, size_t numOfElements)
        {
            ElemPtr& elems = std::get<INDEX> (t);
            elems = static_cast<ElemPtr> (allocator.malloc (sizeof (Elem) * numOfElements));

            Next::malloc (t, allocator, numOfElements);
        }

        static void free (SOAs& t, Allocator& allocator)
        {
            allocator.free (std::get<INDEX> (t));

            Next::free (t, allocator);
        }

        static void memcpy (SOAs& dst, SOAs& src, size_t numOfElements)
        {
            std::memcpy (std::get<INDEX> (dst), std::get<INDEX> (src), sizeof (Elem) * numOfElements);

            Next::memcpy (dst, src, numOfElements);
        }

        static void swap (SOAs& t, size_t a, size_t b)
        {
            ElemPtr elems = std::get<INDEX> (t);
            mdk::swap<Elem> (elems[a], elems[b]);

            Next::swap (t, a, b);
        }
    };

    template<>
    struct Ops<cSize>
    {
        static void reset (SOAs& t)
        {
            (void)t;
        }

        static void malloc (SOAs& t, Allocator& allocator, size_t numOfElements)
        {
            (void)t;
            (void)allocator;
            (void)numOfElements;
        }

        static void free (SOAs& t, Allocator& allocator)
        {
            (void)t;
            (void)allocator;
        }

        static void memcpy (SOAs& dst, SOAs& src, size_t numOfElements)
        {
            (void)dst;
            (void)src;
            (void)numOfElements;
        }

        static void swap (SOAs& t, size_t a, size_t b)
        {
            (void)t;
            (void)a;
            (void)b;
        }
    };

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
#if 0
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
#endif

    //! Returns to if handle is pointing to a memory location within this Manager, otherwise false.
    bool isValid (Handle handle) const;
    
    /*! Get the memory location associated with the given handle.
        Since the memory locations may be changed during resize() or release(), do NOT store the returned pointer for later usages.
     */
    template<size_t INDEX>
    typename ManagerSOA<Manager, INDEX>::ElemPtr get (Handle handle);

#if 0
    /*! Fetch the content of the memory location with synchronization primitive support.
     */
    bool fetch (Data& data, Handle handle) const;

    /*! Store the content to the memory location with synchronization primitive support.
     */
    bool store (Handle handle, const Data& data);
#endif

    /*! Mark the object as enabled.
     */
    void enable (Handle handle);

    /*! Mark the object as disabled.
     */
    void disable (Handle handle);

    /*! Returns true if the associated object is marked as enabled, otherwise false.
     */
    bool isEnabled (Handle handle) const;

#if 0
    /*! Returns a pointer to the first element of all objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    m_inline Data* beginOfAll() const
    {
        return mDataSlot;
    }

    /*! Returns a pointer to the element which follows the last element of all objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    m_inline Data* endOfAll() const
    {
        return mDataSlot + mSize;
    }

    /*! Returns a pointer to the first element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    m_inline Data* beginOfEnabled() const
    {
        return mDataSlot;
    }

    /*! Returns a pointer to the element which follows the last element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    m_inline Data* endOfEnabled() const
    {
        return mDataSlot + mFirstDisabled;
    }

    /*! Returns a pointer to the first element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    m_inline Data* beginOfDisabled() const
    {
        return mDataSlot + mFirstDisabled;
    }

    /*! Returns a pointer to the element which follows the last element of enabled objects.
        This method is provided for compatibility with standard C++ iteration mechanisms.
     */
    m_inline Data* endOfDisabled() const
    {
        return mDataSlot + mSize;
    }
#endif

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
    SOAs mSOAs;      //! slots for holding the actual data
    Handle* mDataHandle;    //! mapping data slot back to the associated Handle, used in copyDataSlot() & swapDataSlot().

    //! Returns true if successfully resized, otherwise false (e.g. out of memory or newCapacity > Handle::kIndexLimit).
    bool resize (size_t newCapacity);

    void swapDataSlot (size_t slotIdxA, size_t slotIdxB);

    m_inline bool isEnabledNoSync (Handle handle) const;

    m_inline bool isValidNoSync (Handle handle) const;

    template<size_t INDEX>
    m_inline typename ManagerSOA<Manager, INDEX>::ElemPtr getNoSync (Handle handle) const;

    m_inline Handle acquireNoSync();

    m_inline bool releaseNoSync (Handle handle);
};

}

#include "mdk_Manager.inl"

#endif  // MDK_MANAGER_H_INCLUDED
