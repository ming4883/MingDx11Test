#include "mdk_Frontend.h"

#if JUCE_WINDOWS
#include "mdk_FrontendWindows.cpp"
#endif

namespace mdk
{

FrontendStartupOptions::FrontendStartupOptions ()
    : width (0)
    , height (0)
    ,fullscreen (false)
{
}

Frontend* Frontend::create (Allocator& allocator)
{
    return m_new<FrontendWinAPI> (allocator);
}

}   // namespace

