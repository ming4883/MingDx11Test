#include "mdk_Frontend.h"
#include <modules/GoogleTest/GoogleTest.h>

using namespace mdk;

TEST (TestFrontend, Basic)
{
    FrontendStartupOptions options;
    options.width = 640;
    options.height = 320;

    Frontend* fn = Frontend::create (CrtAllocator::get());

    fn->startup (options);

    while (fn->update())
    {
        juce::Thread::sleep (1);
    }

    fn->shutdown();

    m_del (CrtAllocator::get(), fn);
}
