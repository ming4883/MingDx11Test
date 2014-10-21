
namespace mdk
{

template<typename TRAITS>
Manager<TRAITS>::Manager (size_t initialCapacity, Allocator& allocator)
    : _allocator (allocator)
    , mCapacity (0)
    , mSize (0)
    , mFirstDisabled (0)
    , mFreelistDequeue (0)
    , mFreelistEnqueue (0)
    , mFreelistCount (0)
    , mATable (nullptr)
    , mDataSlot (nullptr)
    , mDataHandle (nullptr)
{
    SyncWrite sync (mSyncHandle);
    resize (initialCapacity);
}

template<typename TRAITS>
Manager<TRAITS>::~Manager()
{
    {
        SyncWrite sync (mSyncHandle);

        if (mCapacity > 0)
        {
            _allocator.free (mATable);
            _allocator.free (mDataSlot);
            _allocator.free (mDataHandle);
        }

        mCapacity = 0;
        mSize = 0;
        mFirstDisabled = 0;
        mFreelistDequeue = 0;
        mFreelistEnqueue = 0;
        mFreelistCount = 0;
        mATable = nullptr;
        mDataSlot = nullptr;
        mDataHandle = nullptr;
    }
}

template<typename TRAITS>
bool Manager<TRAITS>::resize (size_t newCapacity)
{
    if (newCapacity <= mCapacity)
        return false;

    if (newCapacity > Handle::kIndexLimit)
        return false;

    // allocate memory for new capacity
    ATable* newATable = (ATable*)_allocator.malloc (newCapacity * sizeof (ATable));
    Data* newSlot = (Data*)_allocator.malloc (newCapacity * sizeof (Data));
    Handle* newHandle = (Handle*)_allocator.malloc (newCapacity * sizeof (Handle));

    // initialize the alloation table
    for (size_t i = mCapacity; i < newCapacity; ++i)
    {
        newATable[i].handle.index = (typename TRAITS::HandleDataType)i;
        newATable[i].handle.generation = 1;
        newATable[i].freelistNext = i + 1;
        newATable[i].slotIndex = (size_t)kInvalidSlotIndex;
    }

    if (mCapacity > 0)
    {
        memcpy (newATable, mATable, mCapacity * sizeof (ATable));
        memcpy (newSlot, mDataSlot, mCapacity * sizeof (Data));
        memcpy (newHandle, mDataHandle, mCapacity * sizeof (Handle));

        _allocator.free (mATable);
        _allocator.free (mDataSlot);
        _allocator.free (mDataHandle);

        mFreelistDequeue = mCapacity;
    }

    mATable = newATable;
    mDataSlot = newSlot;
    mDataHandle = newHandle;
    mFreelistCount += newCapacity - mCapacity;
    mFreelistEnqueue = newCapacity - 1;
    mCapacity = newCapacity;

    return true;
}

template<typename TRAITS>
void Manager<TRAITS>::copyDataSlot (size_t from, size_t to)
{
    if (from == to)
        return;

    mATable[mDataHandle[from].index].slotIndex = to;

    //mDataHandle[to] = mDataHandle[from];
    //mDataSlot[to] = mDataSlot[from];

    memcpy (&mDataHandle[to], &mDataHandle[from], sizeof (Handle));
    memcpy (&mDataSlot[to], &mDataSlot[from], sizeof (Data));
}

template<typename TRAITS>
void Manager<TRAITS>::swapDataSlot (size_t a, size_t b)
{
    if (a == b)
        return;

    Data tmpSlot;
    Handle tmpHandle;

    mATable[mDataHandle[a].index].slotIndex = b;
    mATable[mDataHandle[b].index].slotIndex = a;

    tmpHandle = mDataHandle[a];
    mDataHandle[a] = mDataHandle[b];
    mDataHandle[b] = tmpHandle;

    tmpSlot = mDataSlot[a];
    mDataSlot[a] = mDataSlot[b];
    mDataSlot[b] = tmpSlot;
}

template<typename TRAITS>
typename Manager<TRAITS>::Handle Manager<TRAITS>::acquire()
{
    SyncWrite sync (mSyncHandle);
    return acquireNoSync();
}

template<typename TRAITS>
typename Manager<TRAITS>::Handle Manager<TRAITS>::acquireNoSync()
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
        copyDataSlot (mFirstDisabled, mSize);
    }

    ++mFirstDisabled;
    ++mSize;

    mDataHandle[aTable.slotIndex] = aTable.handle;

    return aTable.handle;
}

template<typename TRAITS>
bool Manager<TRAITS>::release (Handle handle)
{
    SyncWrite sync (mSyncHandle);
    return releaseNoSync (handle);
}

template<typename TRAITS>
bool Manager<TRAITS>::releaseNoSync (Handle handle)
{
    if (!isValidNoSync (handle))
        return false;

    ATable& aTable = mATable[handle.index];

    if (isEnabledNoSync (handle))
    {
        copyDataSlot (mFirstDisabled - 1, aTable.slotIndex);
        copyDataSlot (mSize - 1, mFirstDisabled - 1);

        --mSize;
        --mFirstDisabled;
    }
    else
    {
        copyDataSlot (mSize - 1, aTable.slotIndex);
        --mSize;
    }

    // since 0 is a reserved value, we need to handle the overflow of handle.generation
    aTable.handle.generation++;

    if (0 == aTable.handle.generation)
        aTable.handle.generation++;

    // set slotIndex to an invalid value
    aTable.slotIndex = (size_t)kInvalidSlotIndex;

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
bool Manager<TRAITS>::isValid (Handle handle) const
{
    SyncRead sync (mSyncHandle);
    return isValidNoSync (handle);
}

template<typename TRAITS>
bool Manager<TRAITS>::isValidNoSync (Handle handle) const
{
    if (handle.generation == 0)
        return false;

    if (handle.index >= mCapacity)
        return false;

    if (handle.index == kInvalidSlotIndex)
        return false;

    return (handle.generation == mATable[handle.index].handle.generation);
}

template<typename TRAITS>
typename Manager<TRAITS>::Data* Manager<TRAITS>::get (Handle handle)
{
    SyncRead sync (mSyncHandle);
    return getNoSync (handle);
}

template<typename TRAITS>
typename Manager<TRAITS>::Data* Manager<TRAITS>::getNoSync (Handle handle) const
{
    return &mDataSlot[mATable[handle.index].slotIndex];
}

template<typename TRAITS>
bool Manager<TRAITS>::fetch (Data& Data, Handle handle) const
{
    SyncRead sync (mSyncHandle);
    if (!isValidNoSync(handle))
        return false;

    Data = mDataSlot[mATable[handle.index].slotIndex];
    return true;
}

template<typename TRAITS>
bool Manager<TRAITS>::store (Handle handle, const Data& Data)
{
    SyncWrite sync (mSyncHandle);
    if (!isValidNoSync(handle))
        return false;

    mDataSlot[mATable[handle.index].slotIndex] = Data;
    return true;
}

template<typename TRAITS>
void Manager<TRAITS>::enable (Handle handle)
{
    SyncWrite sync (mSyncHandle);

    if (isEnabledNoSync (handle))
        return;

    swapDataSlot (mATable[handle.index].slotIndex, mFirstDisabled);
    ++mFirstDisabled;
}

template<typename TRAITS>
void Manager<TRAITS>::disable (Handle handle)
{
    SyncWrite sync (mSyncHandle);

    if (!isEnabledNoSync (handle))
        return;

    swapDataSlot (mATable[handle.index].slotIndex, mFirstDisabled - 1);
    --mFirstDisabled;
}

template<typename TRAITS>
bool Manager<TRAITS>::isEnabled (Handle handle) const
{
    SyncRead sync (mSyncHandle);
    return isEnabledNoSync (handle);
}

template<typename TRAITS>
bool Manager<TRAITS>::isEnabledNoSync (Handle handle) const
{
    return mATable[handle.index].slotIndex < mFirstDisabled;
}

}
