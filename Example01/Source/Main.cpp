/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"

#include <DirectXMath.h>
using namespace DirectX;

using namespace mdk;

//==============================================================================
class Example01 : public D3D11Demo
{
public:
    Example01()
    {
    }
    
    ~Example01()
    {
        renderThreadStop();
    }

    bool demoStartup()
    {
        return true;
    }

    void demoShutdown()
    {
    }

    void demoUpdate ()
    {
        XMFLOAT4A rgba (0.25f, 0.25f, 1.0f, 1.0f);
        d3dIMContext_->OMSetRenderTargets (1, d3dBackBufRTView_, nullptr);
        d3dIMContext_->ClearRenderTargetView (d3dBackBufRTView_, (float*)&rgba);
        d3dSwapchain_->Present (0u, 0u);
    }
};

//==============================================================================
class Example01Application  : public DemoApplication<Example01>
{
public:
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (Example01Application)
