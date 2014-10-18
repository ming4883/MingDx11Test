#include "mdk_Demo.h"

#include <BinaryData.h>

namespace mdk
{

//==============================================================================
Demo::FPSCameraControl::FPSCameraControl (ThreadPool& threadPool, Camera& camera)
    : threadPool (threadPool), camera (camera), active (true), state (stateIdle)
{
    syncWithCamera();

    job = new JobWithCallback("mdkFPSCameraControl", [this](ThreadPoolJob* self)
    {
        (void)self;

        Vec3f dir = Quat::transform (transform.rotation, moveDir);

        float currTime = (float)Time::getMillisecondCounterHiRes();

        dir = Vec::mul (dir, (currTime - startTime) / 1024.0f);

        transform.position = Vec::add (startTransform.position, dir);
        this->camera.transform.commit (transform);

        startTime = currTime;
        startTransform = transform;

        return (state.get() >= stateMoving) ? ThreadPoolJob::jobNeedsRunningAgain : ThreadPoolJob::jobHasFinished;
    });
}

void Demo::FPSCameraControl::syncWithCamera()
{
    transform = camera.transform.fetch();
}

void Demo::FPSCameraControl::mouseDrag (const MouseEvent& evt)
{
    Point<float> delta = evt.position - startPoint;
    float currTime = (float)Time::getMillisecondCounterHiRes();

    if (state.get() == stateLooking)
    {
        Vec3f rot;
        rot.x = (delta.getY() /-1024.0f) * Scalar::cPI<float>();
        rot.y = (delta.getX() / 1024.0f) * Scalar::cPI<float>();

        Vec4f q = transform.rotation;

        q = Quat::mul (startTransform.rotation, Quat::fromXYZRotation (rot));

        Vec3f dir = Quat::transform (q, Vec3f (0, 0, -1));
        transform.rotation = q;
        Transform::fromLookAt (transform, transform.position, Vec::add (transform.position, dir), Vec3f (0, 1, 0));

        startTransform = transform;
        startPoint = evt.position;
        camera.transform.commit (transform);
    }
    else if (state.get() == stateMoveHori)
    {
        moveDir = Vec3f (-delta.x, 0, delta.y);
        float moveLen = Vec::length (moveDir) * 2.0f;

        if (Scalar::abs (moveDir.x) > Scalar::abs (moveDir.z))
            moveDir = Vec3f (moveLen * Scalar::sign (moveDir.x), 0, 0);
        else
            moveDir = Vec3f (0, 0, moveLen * Scalar::sign (moveDir.z));

        if ((currTime - startTime) > 100.0f)
        {
            //state.set (stateMoving);
        }
    }
    else if (state.get() == stateMoveVert)
    {
        float moveLen = delta.y;
        moveDir = Vec3f (0, moveLen, 0);

        if ((currTime - startTime) > 100.0f)
        {
            //state.set (stateMoving);
        }
    }
}

void Demo::FPSCameraControl::mouseDown (const MouseEvent& evt)
{
    (void)evt;
    if (!active)
        return;

    startTransform = transform;
    startPoint = evt.position;
    startTime = (float)Time::getMillisecondCounterHiRes();

    if (evt.mods.isLeftButtonDown())
    {
        state.set (stateLooking);
    }
    else if (evt.mods.isRightButtonDown())
    {
        moveDir = Vec3f (0, 0, 0);
        state.set (stateMoveHori);
        threadPool.addJob (job, false);
    }
    else if (evt.mods.isMiddleButtonDown())
    {
        moveDir = Vec3f (0, 0, 0);
        state.set (stateMoveVert);
        threadPool.addJob (job, false);
    }
    //m_dprint_begin() << "start dragging..." << m_dprint_end();
}

void Demo::FPSCameraControl::mouseUp (const MouseEvent& evt)
{
    (void)evt;
    if (!active)
        return;

    state.set (stateIdle);
    //m_dprint_begin() << "stop dragging..." << m_dprint_end();
}

//==============================================================================
Demo::Demo()
    : threadPool (1)
{
    appDataAddDir (m_dir_of_cpp().getChildFile ("Media"));
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

void Demo::renderThreadStart (ThreadWithCallback::Function f)
{
    jassert (nullptr == renderThread_);

    renderThread_ = new ThreadWithCallback ("mdkRender", f);
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

void Demo::appDataAddDir (File dir)
{
    if (!dir.isDirectory())
        return;

    appDataDirs.insert (0, dir.getFullPathName());
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

    String* dirBeg = appDataDirs.begin();
    String* dirEnd = appDataDirs.end();
    for (String* dirItr = dirBeg; dirItr != dirEnd; ++dirItr)
    {
        File file = File::createFileWithoutCheckingPath(*dirItr).getChildFile (id);
        
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
    String* dirBeg = appDataDirs.begin();
    String* dirEnd = appDataDirs.end();
    for (String* dirItr = dirBeg; dirItr != dirEnd; ++dirItr)
    {
        File file = File::createFileWithoutCheckingPath(*dirItr).getChildFile (id);
        
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
