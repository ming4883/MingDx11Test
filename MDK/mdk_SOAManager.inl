
namespace mdk
{

template<typename TRAITS>
struct SOAManager<TRAITS>::Handle
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


template<typename TRAITS>
struct SOAManager<TRAITS>::SOAops
{
    enum { cSize = SOAManager::cSOASize };

    template<size_t INDEX>
    static void resetFrom (SOA& t)
    {
        std::get<INDEX> (t) = nullptr;
        resetFrom<INDEX+1> (t);
    }

    template<>
    static void resetFrom<cSize> (SOA&) {}

    template<size_t INDEX>
    static void mallocFrom (SOA& t, Allocator& allocator, size_t numOfElements)
    {
        typedef typename SOAColumn<SOAManager, INDEX>::ElemPtr ElemPtr;
        typedef typename SOAColumn<SOAManager, INDEX>::Elem Elem;

        ElemPtr& elems = std::get<INDEX> (t);
        elems = static_cast<ElemPtr> (allocator.malloc (sizeof (Elem) * numOfElements));

        mallocFrom<INDEX+1> (t, allocator, numOfElements);
    }
    
    template<>
    static void mallocFrom<cSize> (SOA&, Allocator&, size_t) {}

    template<size_t INDEX>
    static void freeFrom (SOA& t, Allocator& allocator)
    {
        allocator.free (std::get<INDEX> (t));

        freeFrom<INDEX+1> (t, allocator);
    }

    template<>
    static void freeFrom<cSize> (SOA&, Allocator&) {}

    template<size_t INDEX>
    static void memcpyFrom (SOA& dst, SOA& src, size_t numOfElements)
    {
        typedef typename SOAColumn<SOAManager, INDEX>::ElemPtr ElemPtr;
        typedef typename SOAColumn<SOAManager, INDEX>::Elem Elem;

        std::memcpy (std::get<INDEX> (dst), std::get<INDEX> (src), sizeof (Elem) * numOfElements);

        memcpyFrom<INDEX+1> (dst, src, numOfElements);
    }
    
    template<>
    static void memcpyFrom<cSize> (SOA&, SOA&, size_t) {}

    template<size_t INDEX>
    static void swapFrom (SOA& t, size_t indexA, size_t indexB)
    {
        typedef typename SOAColumn<SOAManager, INDEX>::ElemPtr ElemPtr;
        typedef typename SOAColumn<SOAManager, INDEX>::Elem Elem;

        ElemPtr& elems = std::get<INDEX> (t);
        mdk::swap<Elem> (elems[indexA], elems[indexB]);

        swapFrom<INDEX+1> (t, indexA, indexB);
    }

    template<>
    static void swapFrom<cSize> (SOA&, size_t, size_t) {}
};

template<typename TRAITS>
SOAManager<TRAITS>::SOAManager (size_t initialCapacity, Allocator& allocator)
    : _allocator (allocator)
    , mCapacity (0)
    , mSize (0)
    , mFirstDisabled (0)
    , mFreelistDequeue (0)
    , mFreelistEnqueue (0)
    , mFreelistCount (0)
    , mATable (nullptr)
    //, mDataSlot (nullptr)
    , mDataHandle (nullptr)
{
    SyncWrite sync (mSyncHandle);
    SOAops::resetFrom<0> (mSOAs);
    resize (initialCapacity);
}

template<typename TRAITS>
SOAManager<TRAITS>::~SOAManager()
{
    {
        SyncWrite sync (mSyncHandle);

        if (mCapacity > 0)
        {
            _allocator.free (mATable);
            //_allocator.free (mDataSlot);
            _allocator.free (mDataHandle);
            SOAops::freeFrom<0> (mSOAs, _allocator);
        }

        mCapacity = 0;
        mSize = 0;
        mFirstDisabled = 0;
        mFreelistDequeue = 0;
        mFreelistEnqueue = 0;
        mFreelistCount = 0;
        mATable = nullptr;
        //mDataSlot = nullptr;
        mDataHandle = nullptr;
    }
}

template<typename TRAITS>
bool SOAManager<TRAITS>::resize (size_t newCapacity)
{
    if (newCapacity <= mCapacity)
        return false;

    if (newCapacity > Handle::cIndexLimit)
        return false;

    // allocate memory for new capacity
    ATable* newATable = (ATable*)_allocator.malloc (newCapacity * sizeof (ATable));
    //Data* newSlot = (Data*)_allocator.malloc (newCapacity * sizeof (Data));
    Handle* newHandle = (Handle*)_allocator.malloc (newCapacity * sizeof (Handle));
    SOA newSOA;
    SOAops::mallocFrom<0> (newSOA, _allocator, newCapacity);

    // initialize the alloation table
    for (size_t i = mCapacity; i < newCapacity; ++i)
    {
        newATable[i].handle.index = (typename TRAITS::HandleDataType)i;
        newATable[i].handle.generation = 1;
        newATable[i].freelistNext = i + 1;
        newATable[i].slotIndex = (size_t)cInvalidSlotIndex;
    }

    if (mCapacity > 0)
    {
        memcpy (newATable, mATable, mCapacity * sizeof (ATable));
        //memcpy (newSlot, mDataSlot, mCapacity * sizeof (Data));
        memcpy (newHandle, mDataHandle, mCapacity * sizeof (Handle));

        SOAops::memcpyFrom<0> (newSOA, mSOAs, mCapacity);

        _allocator.free (mATable);
        //_allocator.free (mDataSlot);
        _allocator.free (mDataHandle);

        SOAops::freeFrom<0> (mSOAs, _allocator);

        mFreelistDequeue = mCapacity;
    }

    mATable = newATable;
    //mDataSlot = newSlot;
    mSOAs = newSOA;
    mDataHandle = newHandle;
    mFreelistCount += newCapacity - mCapacity;
    mFreelistEnqueue = newCapacity - 1;
    mCapacity = newCapacity;

    return true;
}

template<typename TRAITS>
void SOAManager<TRAITS>::swapDataSlot (size_t slotIdxA, size_t slotIdxB)
{
    if (slotIdxA == slotIdxB)
        return;

    mATable[mDataHandle[slotIdxA].index].slotIndex = slotIdxB;
    mATable[mDataHandle[slotIdxB].index].slotIndex = slotIdxA;

    swap<Handle> (mDataHandle[slotIdxA], mDataHandle[slotIdxB]);
    //swap<Data> (mDataSlot[slotIdxA], mDataSlot[slotIdxB]);
    SOAops::swapFrom<0> (mSOAs, slotIdxA, slotIdxB);
}

template<typename TRAITS>
typename SOAManager<TRAITS>::Handle SOAManager<TRAITS>::acquire()
{
    SyncWrite sync (mSyncHandle);
    return acquireNoSync();
}

template<typename TRAITS>
typename SOAManager<TRAITS>::Handle SOAManager<TRAITS>::acquireNoSync()
{
    if (mSize == mCapacity)
    {
        size_t newCapacity = mCapacity * 2;
        if (!resize (newCapacity))
        {
            Handle h;
            h.generation = h.index = 0;
            return h;
        }
    }

    // dequeue from freelist
    ATable& aTable = mATable[mFreelistDequeue];
    mFreelistDequeue = aTable.freelistNext;

    m_assert (mFreelistCount > 0);
    --mFreelistCount;

    aTable.slotIndex = mFirstDisabled;
    if (mFirstDisabled != mSize)
    {
        mATable[mDataHandle[mFirstDisabled].index].slotIndex = mSize;

        mDataHandle[mSize] = mDataHandle[mFirstDisabled];

        //swap<Data> (mDataSlot[mSize], mDataSlot[mFirstDisabled]);
        SOAops::swapFrom<0> (mSOAs, mSize, mFirstDisabled);
    }

    ++mFirstDisabled;
    ++mSize;

    mDataHandle[aTable.slotIndex] = aTable.handle;

    return aTable.handle;
}

template<typename TRAITS>
bool SOAManager<TRAITS>::release (Handle handle)
{
    SyncWrite sync (mSyncHandle);
    return releaseNoSync (handle);
}

template<typename TRAITS>
bool SOAManager<TRAITS>::releaseNoSync (Handle handle)
{
    if (!isValidNoSync (handle))
        return false;

    ATable& aTable = mATable[handle.index];

    if (isEnabledNoSync (handle))
    {
        swapDataSlot (mFirstDisabled - 1, aTable.slotIndex);
        swapDataSlot (mSize - 1, mFirstDisabled - 1);

        --mSize;
        --mFirstDisabled;
    }
    else
    {
        swapDataSlot (mSize - 1, aTable.slotIndex);
        --mSize;
    }

    // since 0 is a reserved value, we need to handle the overflow of handle.generation
    aTable.handle.generation++;

    if (0 == aTable.handle.generation)
        aTable.handle.generation++;

    // set slotIndex to an invalid value
    aTable.slotIndex = (size_t)cInvalidSlotIndex;

    // enqueue to freelist
    mATable[mFreelistEnqueue].freelistNext = handle.index;
    mFreelistEnqueue = handle.index;

    // update mFreelistDequeue if the free list is empty
    if (0 == mFreelistCount)
        mFreelistDequeue = handle.index;

    ++mFreelistCount;

    return true;
}

template<typename TRAITS>
bool SOAManager<TRAITS>::isValid (Handle handle) const
{
    SyncRead sync (mSyncHandle);
    return isValidNoSync (handle);
}

template<typename TRAITS>
bool SOAManager<TRAITS>::isValidNoSync (Handle handle) const
{
    if (handle.generation == 0)
        return false;

    if (handle.index >= mCapacity)
        return false;

    if (handle.index == cInvalidSlotIndex)
        return false;

    return (handle.generation == mATable[handle.index].handle.generation);
}

template<typename TRAITS>
template<typename SOACOL>
typename SOACOL::ElemPtr SOAManager<TRAITS>::get (Handle handle)
{
    SyncRead sync (mSyncHandle);
    return getNoSync<SOACOL> (handle);
}

template<typename TRAITS>
template<typename SOACOL>
typename SOACOL::ElemPtr SOAManager<TRAITS>::getNoSync (Handle handle) const
{
    //return &mDataSlot[mATable[handle.index].slotIndex];
    typename SOACOL::ElemPtr ptr = std::get<SOACOL::cIndex> (mSOAs);
    return &ptr[mATable[handle.index].slotIndex];
}

template<typename TRAITS>
template<typename SOACOL>
bool SOAManager<TRAITS>::fetch (typename SOACOL::Elem& data, Handle handle) const
{
    SyncRead sync (mSyncHandle);
    if (!isValidNoSync (handle))
        return false;

    //data = mDataSlot[mATable[handle.index].slotIndex];
    data = *getNoSync<SOACOL> (handle);
    return true;
}

template<typename TRAITS>
template<typename SOACOL>
bool SOAManager<TRAITS>::store (Handle handle, const typename SOACOL::Elem& data)
{
    SyncWrite sync (mSyncHandle);
    if (!isValidNoSync (handle))
        return false;

    //mDataSlot[mATable[handle.index].slotIndex] = data;
    *getNoSync<SOACOL> (handle) = data;
    return true;
}

template<typename TRAITS>
void SOAManager<TRAITS>::enable (Handle handle)
{
    SyncWrite sync (mSyncHandle);

    if (isEnabledNoSync (handle))
        return;

    swapDataSlot (mATable[handle.index].slotIndex, mFirstDisabled);
    ++mFirstDisabled;
}

template<typename TRAITS>
void SOAManager<TRAITS>::disable (Handle handle)
{
    SyncWrite sync (mSyncHandle);

    if (!isEnabledNoSync (handle))
        return;

    swapDataSlot (mATable[handle.index].slotIndex, mFirstDisabled - 1);
    --mFirstDisabled;
}

template<typename TRAITS>
bool SOAManager<TRAITS>::isEnabled (Handle handle) const
{
    SyncRead sync (mSyncHandle);
    return isEnabledNoSync (handle);
}

template<typename TRAITS>
bool SOAManager<TRAITS>::isEnabledNoSync (Handle handle) const
{
    return mATable[handle.index].slotIndex < mFirstDisabled;
}

}
