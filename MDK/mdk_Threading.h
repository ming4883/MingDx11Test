#ifndef MCD_THREADING_H_INCLUDED
#define MCD_THREADING_H_INCLUDED

#include <AppConfig.h>
#include <modules/juce_core/juce_core.h>

#include <functional>

namespace mdk
{

class ThreadWithCallback : public juce::Thread
{
public:
    typedef std::function<void (Thread*)> Function;
    Function func;

public:
    ThreadWithCallback (const juce::String& threadName, Function f)
        : Thread (threadName)
        , func (f)
    {
    }

    void run() override
    {
        func (this);
    }
};

class JobWithCallback : public juce::ThreadPoolJob
{
public:
    typedef std::function<JobStatus (ThreadPoolJob*)> Function;
    Function func;

    JobWithCallback (const juce::String& jobName, Function f)
        : ThreadPoolJob (jobName)
        , func (f)
    {
    }

    JobStatus runJob() override
    {
        return func (this);
    }
};

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
            juce::Thread::yield();
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

template<typename T>
class Synced
{
public:
    T fetch()
    {
        RSync rsync (sync);
        return value;
    }

    void commit (const T& t)
    {
        WSync wsync (sync);
        value = t;
    }

private:
    typedef SyncWithAtomic Sync;
    typedef ScopedSyncRead<Sync> RSync;
    typedef ScopedSyncWrite<Sync> WSync;
    Sync sync;
    T value;
};

}   // namespace


#endif // MCD_THREADING_H_INCLUDED
