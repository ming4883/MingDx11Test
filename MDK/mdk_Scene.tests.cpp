#include "mdk_Scene.h"

#include <modules/GoogleTest/GoogleTest.h>

using namespace mdk;

class TestAnimationTrackManager : public testing::Test
{
protected:
    AnimationTrackManager manager;

    TestAnimationTrackManager()
        : manager (CrtAllocator::get())
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

#if 0
TEST_F (TestAnimationTrackManager, BasicUsages)
{
    float cFrameTime[] = {0, 1};
    float cFrameData[] = {0, 1, 2, 3, 4, 5, 6, 7};

    // create track
    AnimationTrackHandle handle = manager.acquire();
    EXPECT_TRUE (manager.construct (handle));

    // initialize track contents
    {
        AnimationTrack* track = manager.get (handle);
        track->alloc (2, 4);
        track->setFrameTime (0, 2, cFrameTime);
        track->setFrameData (0, 2, cFrameData);
    }
    
    // make sure enable() and disable() is working
    manager.disable (handle);
    manager.enable (handle);

    // verify track contents
    {
        AnimationTrack* track = manager.get (handle);
        float time[2];
        float data[8];
        track->fetch2Frames (data, time, 0.5f);

        for (int i = 0; i < countof (time); ++i)
            EXPECT_EQ (cFrameTime[i], time[i]);
    }

    // destory track
    EXPECT_TRUE (manager.destruct (handle));
    manager.release (handle);

    EXPECT_FALSE (manager.isValid (handle));
}
#endif

TEST (BenchMark, CpuCacheWrite)
{
    struct RStruct
    {
        Vec3f a, b;
    };

    struct WStruct
    {
        float c;
    };
    
    struct RWStruct
    {
        Vec3f a, b;
        float  c;
    };

    const int N = 1000000;
    const int P = 10;

    RWStruct* rwPtr = (RWStruct*)malloc (sizeof (RWStruct) * N);
    RStruct* rPtr = (RStruct*)malloc (sizeof (RStruct) * N);
    WStruct* wPtr = (WStruct*)malloc (sizeof (WStruct) * N);

    // init values
    for (int n = 0; n < N; ++n)
    {
        rwPtr[n].a = Vec3f ((float)rand(), (float)rand(), (float)rand());
        rwPtr[n].b = Vec3f ((float)rand(), (float)rand(), (float)rand());

        rPtr[n].a = Vec3f ((float)rand(), (float)rand(), (float)rand());
        rPtr[n].b = Vec3f ((float)rand(), (float)rand(), (float)rand());
    }

    TestHelpers::Timer t1;
    TestHelpers::Timer t2;

    for (int p = 0; p < P; ++p)
    {
        // rw on same array
        t1.beginPass();
        for (int n = 0; n < N; ++n)
        {
            rwPtr[n].c = Vec::dot (rwPtr[n].a, rwPtr[n].b);
        }
        t1.endPass();

        {
            float tmp = 0;
            for (int n = 0; n < N; ++n)
                tmp += rwPtr[n].c;

            TestHelpers::doNotOptimizeAway (tmp);
        }
    
        t2.beginPass();
        // rw on separated arrays
        for (int n = 0; n < N; ++n)
        {
            wPtr[n].c = Vec::dot (rPtr[n].a, rPtr[n].b);
        }
        t2.endPass();
   
        {
            float tmp = 0;
            for (int n = 0; n < N; ++n)
                tmp += wPtr[n].c;

            TestHelpers::doNotOptimizeAway (tmp);
        }
    }

    std::stringstream ss;
    ss << t1.average() << ";" << t2.average() << std::endl;

    TestHelpers::log (ss.str());

    free (wPtr);
    free (rPtr);
    free (rwPtr);
}
