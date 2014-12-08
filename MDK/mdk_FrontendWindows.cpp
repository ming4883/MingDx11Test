#include "mdk_FrontendWindows.h"

namespace mdk
{


FrontendWinAPI::FrontendWinAPI (Allocator& allocator)
    : apiHWND_ (nullptr)
    , gfxService_ (allocator)
{
}

FrontendWinAPI::~FrontendWinAPI()
{
}

bool FrontendWinAPI::startup (const FrontendStartupOptions& options)
{
    if (!apiCreateHWND (options.width, options.height, options.fullscreen))
        return false;

    if (!gfxService_.startup (*this))
        return false;
    
    return true;
}

void FrontendWinAPI::shutdown()
{
    DestroyWindow (apiHWND_);
    UnregisterClass (apiWindowClassName(), GetModuleHandle (0));
}

bool FrontendWinAPI::update()
{
    MSG msg = {0};

    if (PeekMessage (&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);

        return msg.message != WM_QUIT;
    }

    return true;
}

GfxService& FrontendWinAPI::getGfxService()
{
    return gfxService_;
}

bool FrontendWinAPI::apiCreateHWND (uint32_t width, uint32_t height, bool fullscreen)
{
    LPCSTR szName = apiWindowClassName();
    WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, apiMsgProc, 0L, 0L, GetModuleHandle (0), 0, 0, 0, 0, szName, 0 };
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED;
    const int kScreenW = GetSystemMetrics (SM_CXSCREEN);
    const int kScreenH = GetSystemMetrics (SM_CYSCREEN);

    // cursor
    wc.hCursor = LoadCursor (0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject (BLACK_BRUSH);
    RegisterClassExA (&wc);
    
    // process parentHWND
    if (fullscreen)
    {
        dwStyle = WS_VISIBLE | WS_POPUP;
        dwExStyle = 0u;
    }

    // process app options
    if (width == 0)
        width = 320;

    if (height == 0)
        height = 568;

    RECT rect;
    SetRect (&rect, 0, 0, width, height);
    AdjustWindowRectEx (&rect, dwStyle, FALSE, dwExStyle);

    int wndW, wndH, wndX, wndY;

    wndW = rect.right - rect.left;
    wndH = rect.bottom - rect.top;
    wndX = kScreenW / 2 - wndW / 2;
    wndY = kScreenH / 2 - wndH / 2;

    apiHWND_ = CreateWindowExA (0, szName, szName, dwStyle, wndX, wndY, wndW, wndH, nullptr, 0, 0, 0);

    return true;
}

LRESULT WINAPI FrontendWinAPI::apiMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
        {
            PostQuitMessage (0);
            break;
        }
        
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

}   // namespace
