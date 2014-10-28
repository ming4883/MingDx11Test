#ifndef MCD_THREADING_H_INCLUDED
#define MCD_THREADING_H_INCLUDED

#include "mdk_Config.h"
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
    m_inline void lockWriter()
    {
    }

    m_inline void unlockWriter()
    {
    }

    m_inline void lockReader()
    {
    }

    m_inline void unlockReader()
    {
    }

    m_inline bool tryLockReader()
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

    m_inline void lockWriter()
    {
        lock();
    }

    m_inline void unlockWriter()
    {
        unlock();
    }

    m_inline void lockReader()
    {
        lock();
    }

    m_inline void unlockReader()
    {
        unlock();
    }

    m_inline bool tryLockReader()
    {
        return tryLock();
    }

private:
    juce::Atomic<int> lockVal_;

    m_inline bool tryLock()
    {
        return 1 == lockVal_.compareAndSetValue (1, 0);
    }

    m_inline void lock()
    {
        while (tryLock())
        {
            juce::Thread::yield();
        }
    }

    m_inline void unlock()
    {
        lockVal_.exchange (0);
    }

};

template<class Sync>
class ScopedSyncRead
{
    m_noncopyable (ScopedSyncRead)

public:
    ScopedSyncRead (Sync& sync) : sync_ (sync)
    {
        sync_.lockReader();
    }

    ~ScopedSyncRead()
    {
        sync_.unlockReader();
    }

private:
    Sync& sync_;
};

template<class Sync>
class ScopedSyncWrite
{
    m_noncopyable (ScopedSyncWrite)

public:
    ScopedSyncWrite (Sync& sync) : sync_ (sync)
    {
        sync_.lockWriter();
    }

    ~ScopedSyncWrite()
    {
        sync_.unlockWriter();
    }

private:
    Sync& sync_;
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
