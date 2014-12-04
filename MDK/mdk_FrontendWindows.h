#ifndef MDK_FRONTENDWINDOWS_H_INCLUDED
#define MDK_FRONTENDWINDOWS_H_INCLUDED

#include "mdk_Frontend.h"

#include "mdk_GraphicsD3D11.h"

namespace mdk
{

class FrontendWinAPI : public Frontend
{
public:
    FrontendWinAPI (Allocator& allocator);
    ~FrontendWinAPI();

    HWND hwnd() { return apiHWND_; }

    bool startup (const FrontendStartupOptions& options) override;
    void shutdown() override;
    bool update() override;

    GfxService& getGfxService() override;

private:
    HWND apiHWND_;
    GfxServiceD3D11 gfxService_;

    bool apiCreateHWND (uint32_t width, uint32_t height, bool fullscreen);

    static LPCSTR apiWindowClassName() { return "MDKWindowClass"; }

    static LRESULT WINAPI apiMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

}

#endif // MDK_FRONTENDWINDOWS_H_INCLUDED
