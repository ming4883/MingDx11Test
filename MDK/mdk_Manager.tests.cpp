#include "mdk_Manager.h"
#include <modules/GoogleTest/GoogleTest.h>

using namespace mdk;

TEST (TestManagerHandle, Basic)
{
    struct Traits : ManagerTraitsDefault<int>
    {
    };

    typedef Manager<Traits>::Handle Handle;
    EXPECT_EQ (sizeof (juce::uint32), sizeof (Handle));

    EXPECT_EQ (1048576, Handle::cIndexLimit);   // 2^20
    EXPECT_EQ (4096, Handle::cGenerationLimit); // 2^12
}

class TestManager : public testing::Test
{
protected:
    struct Object
    {
        int data;
    };

    struct Traits : ManagerTraitsDefault<Object>
    {
    };

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    typedef Manager<Traits> ObjectManager;
};

TEST_F (TestManager, AcquireAndRelease)
{
    const size_t N = 4;
    ObjectManager mgr (N);

    ObjectManager::Handle ha[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    // make sure mgr's capacity is not affected
    EXPECT_EQ (N, mgr.capacity());

    for (int i = 0; i < N; ++i)
    {
        EXPECT_EQ (i, ha[i].index);
        EXPECT_EQ (1, ha[i].generation);
        EXPECT_TRUE (mgr.isValid (ha[i]));

        mgr.get (ha[i])->data = i;
    }

    // test for release, notice that we are destroying in reverse order
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE (mgr.release (ha[i]));

        // h should now be invalid
        EXPECT_FALSE (mgr.isValid (ha[i]));
        EXPECT_FALSE (mgr.release (ha[i]));

        // make sure the content of other allocated object are not affected
        for (int j = i + 1; j < N; ++j)
            EXPECT_EQ (j, mgr.get (ha[j])->data);
    }

    // Acquire the objects again.
    ObjectManager::Handle hb[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    // make sure mgr's capacity is not affected
    EXPECT_EQ (N, mgr.capacity());

    for (int i = 0; i < N; ++i)
    {
        EXPECT_EQ (i, hb[i].index);
        EXPECT_EQ (2, hb[i].generation);
        EXPECT_TRUE (mgr.isValid (hb[i]));
    }
}

TEST_F (TestManager, AllocationContinuity)
{
    ObjectManager mgr (4);

    ObjectManager::Handle ha[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    // make sure the objects are allocated continuously
    for (int i = 1; i < countof (ha); ++i)
    {
        long p0 = reinterpret_cast<long> (mgr.get (ha[i-1]));
        long p1 = reinterpret_cast<long> (mgr.get (ha[i]));

        EXPECT_EQ (sizeof (Object), p1 - p0);
    }
}

TEST_F (TestManager, ResizeSimple)
{
    ObjectManager mgr (4);

    ObjectManager::Handle ha[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    EXPECT_EQ (4, mgr.size());
    EXPECT_EQ (4, mgr.capacity());

    ObjectManager::Handle hb[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    EXPECT_EQ (8, mgr.size());
    EXPECT_EQ (8, mgr.capacity());

    for (int i = 0; i < countof (ha); ++i)
    {
        EXPECT_EQ (i, ha[i].index);
        EXPECT_EQ (1, ha[i].generation);
        EXPECT_TRUE (mgr.isValid (ha[i]));
    }

    for (int i = 0; i < countof (hb); ++i)
    {
        EXPECT_EQ (4+i, hb[i].index);
        EXPECT_EQ (1, hb[i].generation);
        EXPECT_TRUE (mgr.isValid (hb[i]));
    }
}

TEST_F (TestManager, ResizeAdvanced)
{
    ObjectManager mgr (4);

    ObjectManager::Handle ha[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    EXPECT_EQ (4, mgr.capacity());

    for (int i = 0; i < countof (ha); ++i)
        mgr.get (ha[i])->data = i;

    mgr.release (ha[1]);

    ObjectManager::Handle hb[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    for (int i = 0; i < countof (hb); ++i)
        mgr.get (hb[i])->data = 4 + i;

    EXPECT_EQ (7, mgr.size());
    EXPECT_EQ (8, mgr.capacity());

    EXPECT_EQ (0, ha[0].index);
    EXPECT_EQ (1, ha[1].index);
    EXPECT_EQ (2, ha[2].index);
    EXPECT_EQ (3, ha[3].index);

    EXPECT_EQ (1, hb[0].index);
    EXPECT_EQ (4, hb[1].index);
    EXPECT_EQ (5, hb[2].index);
    EXPECT_EQ (6, hb[3].index);

    EXPECT_EQ (1, ha[0].generation);
    EXPECT_EQ (1, ha[1].generation);
    EXPECT_EQ (1, ha[2].generation);
    EXPECT_EQ (1, ha[3].generation);

    EXPECT_EQ (2, hb[0].generation);
    EXPECT_EQ (1, hb[1].generation);
    EXPECT_EQ (1, hb[2].generation);
    EXPECT_EQ (1, hb[3].generation);

    mgr.release (hb[1]);

    EXPECT_EQ (0, mgr.get (ha[0])->data);
    EXPECT_EQ (2, mgr.get (ha[2])->data);
    EXPECT_EQ (3, mgr.get (ha[3])->data);

    EXPECT_EQ (4, mgr.get (hb[0])->data);
    EXPECT_EQ (6, mgr.get (hb[2])->data);
    EXPECT_EQ (7, mgr.get (hb[3])->data);
}

TEST_F (TestManager, EnableAndDisable)
{
    const size_t N = 4;
    ObjectManager mgr (N);

    ObjectManager::Handle ha[] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    for (int i = 0; i < N; ++i)
        mgr.get (ha[i])->data = i;

    // ha[1] should be enabled by default
    EXPECT_TRUE (mgr.isEnabled (ha[1]));

    mgr.disable (ha[1]);
    EXPECT_FALSE (mgr.isEnabled (ha[1]));

    // make sure the contents are not affected by disable(ha[1])
    for (int i = 0; i < N; ++i)
        EXPECT_EQ (i, mgr.get (ha[i])->data);

    mgr.enable (ha[1]);
    EXPECT_TRUE (mgr.isEnabled (ha[1]));

    // make sure the contents are not affected by enable(ha[1])
    for (int i = 0; i < N; ++i)
        EXPECT_EQ (i, mgr.get (ha[i])->data);
}

TEST_F (TestManager, AcquireWithDisable)
{
    const size_t N = 4;
    ObjectManager mgr (N);

    ObjectManager::Handle ha[N * 2] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    for (int i = 0; i < N; ++i)
        mgr.get (ha[i])->data = i;

    mgr.disable (ha[1]);

    ha[4] = mgr.acquire();
    ha[5] = mgr.acquire();
    ha[6] = mgr.acquire();
    ha[7] = mgr.acquire();

    for (int i = N; i < 2 * N; ++i)
        mgr.get (ha[i])->data = i;

    for (int i = 0; i < 2 * N; ++i)
        EXPECT_EQ (i, mgr.get (ha[i])->data);

}

TEST_F (TestManager, ReleaseEnabled)
{
    const size_t N = 4;
    ObjectManager mgr (N);

    ObjectManager::Handle ha[N] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    for (int i = 0; i < N; ++i)
        mgr.get (ha[i])->data = i;

    mgr.disable (ha[1]);

    mgr.release (ha[0]);

    // make sure the contents are not affected by .release (ha[0])
    EXPECT_EQ (1, mgr.get (ha[1])->data);
    EXPECT_EQ (2, mgr.get (ha[2])->data);
    EXPECT_EQ (3, mgr.get (ha[3])->data);
}

TEST_F (TestManager, ReleaseDisabled)
{
    const size_t N = 4;
    ObjectManager mgr (N);

    ObjectManager::Handle ha[N] =
    {
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
        mgr.acquire(),
    };

    for (int i = 0; i < N; ++i)
        mgr.get (ha[i])->data = i;

    mgr.disable (ha[1]);

    mgr.release (ha[1]);

    // make sure the contents are not affected by .release (ha[0])
    EXPECT_EQ (0, mgr.get (ha[0])->data);
    EXPECT_EQ (2, mgr.get (ha[2])->data);
    EXPECT_EQ (3, mgr.get (ha[3])->data);
}


TEST_F (TestManager, MultithreadAcquire)
{
    ObjectManager mgr (4096);

    ObjectManager::Handle h[4096];

    auto f1 = [&h, &mgr]()
    {
        Object obj;
        int BEG = 0;
        int END = 1024;

        for (int i = BEG; i < END; ++i)
        {
            obj.data = i;
            h[i] = mgr.acquire();
            mgr.store (h[i], obj);
            if ((i % 2) == 0)
                mgr.disable (h[i]);
        }
    };

    auto f2 = [&h, &mgr]()
    {
        Object obj;
        int BEG = 1024;
        int END = 2048;

        for (int i = BEG; i < END; ++i)
        {
            obj.data = i;
            h[i] = mgr.acquire();
            mgr.store (h[i], obj);
            if ((i % 2) == 0)
                mgr.disable (h[i]);
        }
    };

    auto f3 = [&h, &mgr]()
    {
        Object obj;
        int BEG = 2048;
        int END = 3072;

        for (int i = BEG; i < END; ++i)
        {
            obj.data = i;
            h[i] = mgr.acquire();
            mgr.store (h[i], obj);
            if ((i % 2) == 0)
                mgr.disable (h[i]);
        }
    };

    auto f4 = [&h, &mgr]()
    {
        Object obj;
        int BEG = 3072;
        int END = 4096;

        for (int i = BEG; i < END; ++i)
        {
            obj.data = i;
            h[i] = mgr.acquire();
            mgr.store (h[i], obj);
            if ((i % 2) == 0)
                mgr.disable (h[i]);
        }
    };

    long t1 = TestHelpers::startThread (f1);
    long t2 = TestHelpers::startThread (f2);
    long t3 = TestHelpers::startThread (f3);
    long t4 = TestHelpers::startThread (f4);

    TestHelpers::waitForThreadExit (t1);
    TestHelpers::waitForThreadExit (t2);
    TestHelpers::waitForThreadExit (t3);
    TestHelpers::waitForThreadExit (t4);

    for (int i = 0; i < 4096; ++i)
    {
        Object obj;
        mgr.fetch (obj, h[i]);
        EXPECT_EQ (i, obj.data);
        EXPECT_EQ ((i % 2) == 0, !mgr.isEnabled (h[i]));
    }
}