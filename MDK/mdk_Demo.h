#ifndef MDK_DEMO_H_INCLUDED
#define MDK_DEMO_H_INCLUDED

#include <AppConfig.h>
#include <modules/juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "mdk_Math.h"
#include "mdk_SyncPrimitive.h"

using namespace juce;

#define m_stringify(x) #x
#define m_tostr(x) m_stringify(x)

#define m_isnull(x) reportTrue (x == nullptr, __FILE__ "(" m_tostr(__LINE__) "): " #x)

#define m_log_begin() { juce::String _; _
#define m_log_end() ""; juce::Logger::writeToLog (_); }

#define m_dprint_begin() { juce::String _; _
#define m_dprint_end() ""; juce::Logger::outputDebugString (_); }

namespace mdk
{

class Demo : public Component
{
public:
    Demo();
    virtual ~Demo();

    virtual bool requestShutdown();

    virtual bool demoStartup() = 0;
    virtual void demoUpdate() = 0;
    virtual void demoShutdown() = 0;

    inline bool reportTrue (bool boolVal, const char* errMsg)
    {
        if (true == boolVal)
            errors_.add (String (errMsg));

        return boolVal;
    }

    void appDataAddDir (File dir);

    const char* appDataGet (const char* id, int& size);

    InputStream* appDataGet (const char* id);

public:

// Mouse
    class MouseListenerWithCallback : public MouseListener
    {
    public:
        typedef std::function<void (const MouseEvent& )> MouseCallback;

        MouseCallback onMouseMove;
        MouseCallback onMouseDown;
        MouseCallback onMouseUp;
        MouseCallback onMouseDrag;
        
        void mouseMove (const MouseEvent& event) override
        {
            if (onMouseMove)
                onMouseMove (event);
        }

        void mouseDown (const MouseEvent& event) override
        {
            if (onMouseDown)
                onMouseDown (event);
        }
    
        void mouseDrag (const MouseEvent& event) override
        {
            if (onMouseDrag)
                onMouseDrag (event);
        }
    
        void mouseUp (const MouseEvent& event) override
        {
            if (onMouseUp)
                onMouseUp (event);
        }
    };
    
// Camera
    class Camera
    {
    public:
        struct Projection
        {
            float fovY;
            float aspect;
            float zNear;
            float zFar;
        };

        Synced<Transform3f> transform;
        Synced<Projection> projection;

        Mat44f viewMatrix;              //!< derived from transform
        Mat44f projectionMatrix;        //!< derived from projection
        Mat44f projectionviewMatrix;    //!< derived from viewMatrix and viewprojectionMatrix

        Camera();

        void updateD3D (float rtAspect = 1.0f);
    };

protected:

// RenderThread
    class RenderThread : public Thread
    {
    public:
        typedef std::function<void (Thread*)> Function;
        Function func;
        RenderThread (Function f);
        void run() override;
    };

    bool renderThreadExists() const;

    void renderThreadStart (RenderThread::Function f);

    void renderThreadStop();

// AppData
    struct AppData
    {
        MemoryBlock data;
    };

    typedef HashMap<int, AppData*> AppDataCache;
    
    StringArray appDataDirs;

    AppDataCache appDataCache;

    void appDataReset();


// error report
    inline void errClear()
    {
        errors_.clearQuick();
    }

    inline void errLog (const char* errMsg)
    {
        if (nullptr == errMsg)
            return;

        errors_.add (String (errMsg));
    }

    inline void errDump()
    {
        for (int i = 0; i < errors_.size(); ++i)
            Logger::outputDebugString (errors_.getReference(i) + "\n");
    }

    class ErrorReporter
    {
        Demo* demo;
    public:
        ErrorReporter (Demo* d)
            : demo (d)
        {
            demo->errClear();
        }

        ~ErrorReporter()
        {
            demo->errDump();
        }
    };

// Time
    inline void timeInit()
    {
        timeLastFrame_ = Time::getMillisecondCounterHiRes();
        timeDelta_= 0;
        timeSmoothDelta_ = 0;
        timeAccum_ = 0;
    }

    inline void timeUpdate()
    {
        double timeThisFrame = Time::getMillisecondCounterHiRes();
        
        double delta = timeThisFrame - timeLastFrame_;

        if (0 == timeDelta_)
        {
            timeSmoothDelta_ = delta;
        }
        else
        {
            const double a0 = 0.25;
            const double a1 = 1.0 - a0;
            timeSmoothDelta_ = delta * a0 + timeSmoothDelta_ * a1;
        }

        timeDelta_ = delta;
        timeAccum_ += delta;

        timeLastFrame_ = timeThisFrame;
    }

    template<typename REAL = double>
    inline REAL timeGetDeltaMS() const
    {
        return (REAL)timeDelta_;
    }

    template<typename REAL = double>
    inline REAL timeGetSmoothDeltaMS() const
    {
        return (REAL)timeSmoothDelta_;
    }
    
    template<typename REAL = double>
    inline REAL timeGetAccumMS() const
    {
        return (REAL)timeAccum_;
    }

private:
    StringArray errors_;
    ScopedPointer<RenderThread> renderThread_;
    double timeLastFrame_;
    double timeDelta_;
    double timeSmoothDelta_;
    double timeAccum_;
};

class DemoWindow : public DocumentWindow
{
public:
    DemoWindow (Demo* demo, const String& cmdLine);

    void userTriedToCloseWindow() override;

    void closeButtonPressed() override;

private:
    Demo* demo_;

    void tryQuitting();
};

template<class DEMO>
class DemoApplication : public JUCEApplication
{
public:
    ScopedPointer<DemoWindow> window;

    DemoApplication () {}
    virtual ~DemoApplication() {}

    void initialise (const String& cmdLine) override
    {
        window = new DemoWindow (new DEMO, cmdLine);
        window->setVisible (true);
    }

    void shutdown() override
    {
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& ) override
    {
    }
};

} // namespace

#define m_dir_of_cpp() File(__FILE__).getParentDirectory()

#endif	// MDK_DEMO_H_INCLUDED
