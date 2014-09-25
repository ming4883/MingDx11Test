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
Demo::Camera::Camera()
{
    projection.aspect = 1.0f;
    projection.fovY = Scalar<float>::rad (45.0f);
    projection.zFar = 1025.0f;
    projection.zNear = 1.0f;

    Transform::setIdentity (transform);
}

void Demo::Camera::updateD3D (float rtAspect)
{
    Transform3<float> inv;
    Transform::inverse (inv, transform);
    Mat::fromTransform3 (viewMatrix, inv);

    float fovyDiv2 = projection.fovY * 0.5f;
    float aspect = rtAspect * projection.aspect;
    float zn = projection.zNear;
    float zf = projection.zFar;
    float yscale = std::cosf (fovyDiv2) / std::sinf (fovyDiv2);
    float xscale = yscale / aspect;
    float zscale = 1.0f / (zn - zf);

    projectionMatrix[0][0] = xscale;
    projectionMatrix[0][1] = 0;
    projectionMatrix[0][2] = 0;
    projectionMatrix[0][3] = 0;

    projectionMatrix[1][0] = 0;
    projectionMatrix[1][1] = yscale;
    projectionMatrix[1][2] = 0;
    projectionMatrix[1][3] = 0;

    projectionMatrix[2][0] = 0;
    projectionMatrix[2][1] = 0;
    projectionMatrix[2][2] = zf * zscale;
    projectionMatrix[2][3] = zn * zf * zscale;

    projectionMatrix[3][0] = 0;
    projectionMatrix[3][1] = 0;
    projectionMatrix[3][2] =-1;
    projectionMatrix[3][3] = 0;
}

//==============================================================================
Demo::Demo()
{
}

Demo::~Demo()
{
    appDataReset();
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

void Demo::appDataReset()
{
    AppDataCache::Iterator iter (appDataCache);

    while (iter.next())
        delete iter.getValue();
    appDataCache.clear();
}

const char* Demo::appDataGet (const char* id, int& size)
{
    String fileId = String (id).replaceCharacter ('.', '_');

    int hash = fileId.hashCode();

    // check for cache
    if (appDataCache.contains(hash))
    {
        AppData* appData = appDataCache[hash];
        size = appData->data.getSize();
        return (char*)appData->data.getData();
    }

    if (appDataDir.isNotEmpty())
    {
        File file = File::createFileWithoutCheckingPath(appDataDir).getChildFile (id);
        
        if (file.existsAsFile())
        {
            AppData* appData = new AppData;
            FileInputStream stream (file);
            stream.readIntoMemoryBlock (appData->data);
            
            appDataCache.set (hash, appData);

            size = appData->data.getSize();
            return (char*)appData->data.getData();
        }
    }

    return BinaryData::getNamedResource (fileId.toRawUTF8(), size);
}

InputStream* Demo::appDataGet (const char* id)
{
    if (appDataDir.isNotEmpty())
    {
        File file = File::createFileWithoutCheckingPath(appDataDir).getChildFile (id);
        
        if (file.existsAsFile())
        {
            return new FileInputStream (file);
        }
    }

    int dataSize = 0;
    const char* data = appDataGet (id, dataSize);

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
