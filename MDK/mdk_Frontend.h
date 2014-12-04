#ifndef MDK_FRONTEND_H_INCLUDED
#define MDK_FRONTEND_H_INCLUDED

#include "mdk_Config.h"
#include "mdk_Allocator.h"

namespace mdk
{

class GfxService;

class FrontendStartupOptions
{
public:
    uint32_t width;
    uint32_t height;
    bool fullscreen;

    FrontendStartupOptions ();
};

class Frontend
{
    m_noncopyable (Frontend)

public:
    static Frontend* create (Allocator& allocator);
    
    Frontend() {}
    virtual ~Frontend() {}

    virtual bool startup (const FrontendStartupOptions& options) = 0;
    virtual void shutdown() = 0;
    virtual bool update() = 0;

    virtual GfxService& getGfxService() = 0;
};

}

#endif // MDK_FRONTEND_H_INCLUDED
