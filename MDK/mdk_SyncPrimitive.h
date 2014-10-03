#ifndef MCD_SYNCPRIMITIVE_H_INCLUDED
#define MCD_SYNCPRIMITIVE_H_INCLUDED

#include <modules/juce_core/juce_core.h>

namespace mdk
{

class SyncWithNull
{
public:
    inline void lockWriter ()
    {
    }

    inline void unlockWriter ()
    {
    }

    inline void lockReader ()
    {
    }

    inline void unlockReader ()
    {
    }

    inline bool tryLockReader ()
    {
        return true;
    }
};

class SyncWithAtomic
{
public:
    SyncWithAtomic() : lockVal_ (0)
    {
    }

    inline void lockWriter ()
    {
        lock();
    }

    inline void unlockWriter ()
    {
        unlock();
    }

    inline void lockReader ()
    {
        lock();
    }

    inline void unlockReader ()
    {
        unlock();
    }

    inline bool tryLockReader ()
    {
        return tryLock();
    }

private:
    juce::Atomic<int> lockVal_;

    inline bool tryLock()
    {
        return 1 == lockVal_.compareAndSetValue (1, 0);
    }

    inline void lock()
    {
        while (tryLock())
        {
            Thread::yield();
        }
    }

    inline void unlock()
    {
        lockVal_.exchange (0);
    }

};

template<class Sync>
class ScopedSyncRead
{
    Sync& sync_;

    ScopedSyncRead (const ScopedSyncRead& source);
    void operator = (const ScopedSyncRead& source);

public:
    ScopedSyncRead (Sync& sync) : sync_ (sync)
    {
        sync_.lockReader();
    }

    ~ScopedSyncRead ()
    {
        sync_.unlockReader();
    }
};

template<class Sync>
class ScopedSyncWrite
{
    Sync& sync_;

    ScopedSyncWrite (const ScopedSyncWrite& source);
    void operator = (const ScopedSyncWrite& source);

public:
    ScopedSyncWrite (Sync& sync) : sync_ (sync)
    {
        sync_.lockWriter();
    }

    ~ScopedSyncWrite ()
    {
        sync_.unlockWriter();
    }
};

}   // namespace


#endif // MCD_SYNCPRIMITIVE_H_INCLUDED
