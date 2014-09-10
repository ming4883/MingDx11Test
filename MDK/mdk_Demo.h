#ifndef MDK_DEMO_H_INCLUDED
#define MDK_DEMO_H_INCLUDED

#include <AppConfig.h>
#include <modules/juce_gui_basics/juce_gui_basics.h>
#include <functional>

using namespace juce;

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


protected:
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

    inline bool reportIfFalse (bool okay, const char* errMsg)
    {
        if (!okay)
        {
            errors_.add (String (errMsg));
        }

        return okay;
    }

    inline void errClear()
    {
        errors_.clearQuick();
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

private:
    StringArray errors_;
    ScopedPointer<RenderThread> renderThread_;
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

#endif	// MDK_DEMO_H_INCLUDED
