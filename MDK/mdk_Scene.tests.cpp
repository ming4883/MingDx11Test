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

    // destory track
    EXPECT_TRUE (manager.destruct (handle));
    manager.release (handle);

    EXPECT_FALSE (manager.isValid (handle));
}
