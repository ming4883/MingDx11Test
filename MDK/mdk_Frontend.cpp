#include "mdk_Frontend.h"

#include "mdk_FrontendWindows.cpp"

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
    return m_new (allocator, FrontendWinAPI) (allocator);
}

}   // namespace

