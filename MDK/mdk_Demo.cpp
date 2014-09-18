#include "mdk_Demo.h"

#include <BinaryData.h>

namespace mdk
{

//==============================================================================
Demo::RenderThread::RenderThread(Function f)
    : Thread ("Render")
    , func (f)
{
}

void Demo::RenderThread::run()
{
    func (this);
}

//==============================================================================
Demo::Demo()
{
}

Demo::~Demo()
{
}

bool Demo::requestShutdown()
{
    renderThreadStop();
    return true;
}

bool Demo::renderThreadExists() const
{
    return renderThread_ != nullptr;
}

void Demo::renderThreadStart (RenderThread::Function f)
{
    jassert (nullptr == renderThread_);

    renderThread_ = new RenderThread (f);
    renderThread_->startThread();
}

void Demo::renderThreadStop()
{
    if (renderThread_)
    {
        renderThread_->signalThreadShouldExit();
        renderThread_->waitForThreadToExit (-1);
        renderThread_ = nullptr;
    }
}

const char* Demo::binDataGet (const char* id, int& size)
{
    String fileId = String (id).replaceCharacter ('.', '_');
    return BinaryData::getNamedResource (fileId.toRawUTF8(), size);
}

InputStream* Demo::binDataGet (const char* id)
{
    int dataSize = 0;
    const char* data = binDataGet (id, dataSize);

    if (!data)
        return nullptr;

    return new MemoryInputStream (data, dataSize, false);
}

//==============================================================================
DemoWindow::DemoWindow (Demo* demo, const String& /*cmdLine*/)
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(), LookAndFeel::getDefaultLookAndFeel().findColour (DocumentWindow::backgroundColourId), DocumentWindow::allButtons, true)
    , demo_ (demo)
{
    jassert (demo_);
    setUsingNativeTitleBar (true);
    BorderSize<int> border = getBorderThickness();
            
#if JUCE_IOS || JUCE_ANDROID
    int w = int (Desktop::getInstance().getDisplays().getMainDisplay().userArea.getWidth());
    int h = int (Desktop::getInstance().getDisplays().getMainDisplay().userArea.getHeight());
#else
    int w = 1280;
    int h = 720;
#endif

    centreWithSize (w + border.getLeftAndRight(), h + border.getTopAndBottom());

    setContentOwned (demo_, false);
}

void DemoWindow::userTriedToCloseWindow()
{
    tryQuitting();
}

void DemoWindow::closeButtonPressed()
{
    tryQuitting();
}

void DemoWindow::tryQuitting()
{
    jassert (demo_);
    if (demo_->requestShutdown())
        JUCEApplication::quit();
}

} // namespace
