#if MDK_TEST_FRONTEND

#include "mdk_Frontend.h"
#include <modules/GoogleTest/GoogleTest.h>

using namespace mdk;

class TestFrontend : public testing::Test
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

TEST_F (TestFrontend, Basic)
{
    Frontend* fn = Frontend::create (CrtAllocator::get());

    fn->startup (startupOptions);

    while (fn->update())
    {
        TestHelpers::sleep (1);
    }

    fn->shutdown();

    m_del (CrtAllocator::get(), fn);
}

#endif
