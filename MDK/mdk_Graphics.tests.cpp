#if MDK_TEST_GRAPHICS

#include "mdk_Frontend.h"
#include <modules/GoogleTest/GoogleTest.h>

using namespace mdk;

class TestGraphics : public testing::Test
{
protected:
    
    void SetUp() override
    {
        startupOptions.width = 640;
        startupOptions.height = 320;
    }

    void TearDown() override
    {
    }

    FrontendStartupOptions startupOptions;

};


TEST_F (TestGraphics, Buffers)
{
    Frontend* fn = Frontend::create (CrtAllocator::get());
    fn->startup (startupOptions);

    GfxService* gfx = &fn->getGfxService();

    fn->shutdown();

    m_del (fn);
}

TEST_F (TestGraphics, FrameBeginEnd)
{
    Frontend* fn = Frontend::create (CrtAllocator::get());
    fn->startup (startupOptions);

    GfxService* gfx = &fn->getGfxService();

    while (fn->update())
    {
        gfx->frameBegin();

        gfx->colorTargetClear (gfx->colorTargetDefault(), 0.25f, 0.25f, 1.0f, 1.0f);
        gfx->depthTargetClear (gfx->depthTargetDefault(), 1.0f);

        gfx->frameEnd();
        TestHelpers::sleep (1);
    }

    fn->shutdown();

    m_del (fn);
}

#endif
