#include "GoogleTest.h"

// The following lines pull in the real gtest *.cc files.
#include "src/gtest.cc"
#include "src/gtest-death-test.cc"
#include "src/gtest-filepath.cc"
#include "src/gtest-port.cc"
#include "src/gtest-printers.cc"
#include "src/gtest-test-part.cc"
#include "src/gtest-typed-test.cc"

#if 0
#include "src/gtest_main.cc"

#else

#include <AppConfig.h>
#include <modules/juce_gui_basics/juce_gui_basics.h>

using namespace juce;

#ifndef JUCE_GOOGLE_TEST_NAME
#define JUCE_GOOGLE_TEST_NAME GoogleTest
#endif

#define STR(x) #x
#define STRINGIFY(x) STR(x)

//==============================================================================
class ThisApplication  : public JUCEApplication
{
private:

    class MainView : public Component, public Timer
    {
    public:
        MainView ()
        {
            addAndMakeVisible (textEditor_);
            textEditor_.setReadOnly (true);
            textEditor_.setMultiLine (true);
            
#if JUCE_IOS || JUCE_ANDROID
            textEditor_.setFont (Font ("Courier New", 9.0f, Font::bold));
#else
            textEditor_.setFont (Font ("Courier New", 12.0f, Font::bold));
#endif
            startTimer (500);
        }

        void resized() override
        {
            textEditor_.setBounds (2, 2, getWidth() - 4, getHeight() - 4);
        }

        void log (bool iserror, const String& str)
        {
            ScopedWriteLock wlock (rwlock_);
            logItems_.add (LogItem (iserror, str));
        }

        void timerCallback() override
        {
            ScopedWriteLock wlock (rwlock_);

            const int kCnt = logItems_.size();

            if (0 == kCnt)
                return;

            bool iserrorLast = !logItems_.getRawDataPointer()->iserror;

            textEditor_.moveCaretToEnd();

            for (int i = 0; i < kCnt; ++i)
            {
                const LogItem& item = logItems_.getReference (i);

                if (iserrorLast != item.iserror)
                {
                    textEditor_.setColour (TextEditor::textColourId, item.iserror ? Colours::red : Colours::green);
                    iserrorLast = item.iserror;
                }

                textEditor_.insertTextAtCaret (item.str);
            }

            logItems_.clear();
        }

    private:
        class LogItem
        {
        public:
            bool iserror;
            String str;

            LogItem (bool iserror, const String& str) : iserror (iserror), str(str)
            {
            }
        };

        TextEditor textEditor_;
        Array<LogItem> logItems_;
        ReadWriteLock rwlock_;
    };

    class TestListener : public ::testing::EmptyTestEventListener
    {
    public:
        MainView::SafePointer<MainView> mainView;

        TestListener (MainView* mainView) : mainView (mainView)
        {
        }

        static String fmtTestCnt (int i)
        {
            String ret;
            ret << i;
            if (i > 1)
                ret << " tests";
            else
                ret << " test";

            return ret;
        }

        static String fmtTestCaseCnt (int i)
        {
            String ret;
            ret << i;
            if (i > 1)
                ret << " test cases";
            else
                ret << " test case";

            return ret;
        }

        void OnTestIterationStart (const ::testing::UnitTest& unit_test, int iter) override
        {
            if (nullptr == mainView)
                return;

            String msg;
            msg << "\n";
            msg << "[==========] ";
            msg << "Running " << fmtTestCnt (unit_test.test_to_run_count()) << " from " << fmtTestCaseCnt (unit_test.test_case_to_run_count());
            msg << " (iteration " << (iter + 1) << ")";
            msg << "\n";

            mainView->log (false, msg);
        }

        void OnTestIterationEnd (const ::testing::UnitTest& unit_test, int) override
        {
            if (nullptr == mainView)
                return;

            String msg;

            msg << "[==========] ";
            msg << fmtTestCnt (unit_test.test_to_run_count()) << " from " << fmtTestCaseCnt (unit_test.test_case_to_run_count()) << " ran.";

            if (::testing::GTEST_FLAG (print_time))
            {
                msg << " (" << unit_test.elapsed_time() << " ms total)";
            }
            msg << "\n";
            msg << "[  PASSED  ] " << fmtTestCnt (unit_test.successful_test_count()) << "\n";

            mainView->log (false, msg);

            if (!unit_test.Passed())
            {
                msg.clear();
                const int failed_test_count = unit_test.failed_test_count();
                msg << "[  FAILED  ] " << fmtTestCnt (failed_test_count) << "\n";

                mainView->log (true, msg);
            }
        }

        void OnTestCaseStart (const ::testing::TestCase& test_case) override
        {
            if (nullptr == mainView)
                return;

            juce::String msg;
            msg << "\n";
            msg << "[----------] " << fmtTestCnt (test_case.test_to_run_count()) << " from " << test_case.name() << "\n";
            mainView->log (false, msg);
        }

        void OnTestCaseEnd (const ::testing::TestCase& test_case) override
        {
            if (nullptr == mainView)
                return;

            juce::String msg;
            msg << "[----------] " << fmtTestCnt (test_case.test_to_run_count()) << " from " << test_case.name();

            if (::testing::GTEST_FLAG(print_time))
                msg << " (" << test_case.elapsed_time() << ") ms";

            msg << "\n";
            msg << "\n";
            mainView->log (false, msg);
        }

        void OnTestPartResult(const ::testing::TestPartResult& result) override
        {
            if (result.type() == ::testing::TestPartResult::kSuccess)
                return;

            if (nullptr == mainView)
                return;

            const char* file_name = result.file_name();

            file_name = (nullptr == file_name) ? "unknown file" : file_name;

            juce::String msg;
            msg << "\nat " << file_name << ":" << result.line_number() << ": Failure\n";
            msg << result.summary() << "\n";

            mainView->log (result.failed(), msg);
        }
        
        void OnTestStart (const ::testing::TestInfo& test_info)
        {
            if (nullptr == mainView)
                return;
            
            const testing::TestResult& test_result = *test_info.result();
            
            juce::String msg;
            msg << "[          ] " << test_info.name() << "\n";
            
            mainView->log (test_result.Failed(), msg);
        }

        void OnTestEnd (const ::testing::TestInfo& test_info) override
        {
            if (nullptr == mainView)
                return;

            const testing::TestResult& test_result = *test_info.result();

            juce::String msg;

            if (test_result.Passed())
                msg << "[       OK ] ";
            else if (test_result.HasNonfatalFailure())
                msg << "[     FAIL ] ";
            else
                msg << "[    FATAL ] ";

            msg << test_info.name();

            if (::testing::GTEST_FLAG(print_time))
                msg << " (" << test_result.elapsed_time() << ") ms";

            msg << "\n";
            
            mainView->log (test_result.Failed(), msg);
        }

    };

    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow (const juce::String& name) : DocumentWindow (name, LookAndFeel::getDefaultLookAndFeel().findColour (DocumentWindow::backgroundColourId), DocumentWindow::allButtons, true)
        {
            setUsingNativeTitleBar (true);
            BorderSize<int> border = getBorderThickness();
            
#if JUCE_IOS || JUCE_ANDROID
            int w = int (Desktop::getInstance().getDisplays().getMainDisplay().userArea.getWidth());
            int h = int (Desktop::getInstance().getDisplays().getMainDisplay().userArea.getHeight());
#else
            int w = int (Desktop::getInstance().getDisplays().getMainDisplay().userArea.getWidth() * 0.875);
            int h = int (Desktop::getInstance().getDisplays().getMainDisplay().userArea.getHeight() * 0.875);
#endif

            centreWithSize (w + border.getLeftAndRight(), h + border.getTopAndBottom());
        }

        void userTriedToCloseWindow() override
        {
            JUCEApplication::quit();
        }

        void closeButtonPressed() override
        {
            JUCEApplication::quit();
        }
    };

    class GoogleTestThread : public juce::Thread
    {
    public:
        StringArray cmds;
        TestListener* testListener;

        GoogleTestThread() : juce::Thread ("GoogleTest")
        {
        }

        ~GoogleTestThread()
        {
        }

        void run() override
        {
            juce::JUCEApplication* app = juce::JUCEApplication::getInstance();

            int argc = cmds.size() + 1;
            char** argv = new char*[argc];
            argv[0] = app->getApplicationName().getCharPointer().getAddress();

            for (int i = 0; i < cmds.size(); ++i)
                argv[i + 1] = cmds[i].getCharPointer().getAddress();

            ::testing::InitGoogleTest (&argc, argv);

            ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
            listeners.Append(testListener);
            //delete listeners.Release(listeners.default_result_printer());

            app->setApplicationReturnValue (RUN_ALL_TESTS());

            delete[] argv;
        }
    };

public:
    //==============================================================================
    ThisApplication()
    {
    }

    const String getApplicationName()       { return STRINGIFY (JUCE_GOOGLE_TEST_NAME)" - GoogleTest"; }
    const String getApplicationVersion()    { return "1.7.0"; }
    bool moreThanOneInstanceAllowed()       { return true; }

    //==============================================================================
    void initialise (const String& commandLine)
    {
        LookAndFeel::setDefaultLookAndFeel (&lookNFeel_);

        mainView_ = new MainView();

        mainWindow_ = new MainWindow (STRINGIFY (JUCE_GOOGLE_TEST_NAME));
        mainWindow_->setContentOwned (mainView_, false);
        mainWindow_->setVisible (true);

        thread_ = new GoogleTestThread;
        thread_->cmds.addTokens (commandLine, true);
        thread_->testListener = new TestListener (mainView_);

        thread_->startThread();
    }

    void shutdown()
    {
        thread_->stopThread (-1);
        delete thread_;

        mainWindow_ = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit()
    {
        quit();
    }

    void anotherInstanceStarted (const String& /*commandLine*/)
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }
    
    MainView& mainView() { return *mainView_; }
    
private:
    LookAndFeel_V3 lookNFeel_;
    ScopedPointer<MainWindow> mainWindow_;
    MainView* mainView_;
    GoogleTestThread* thread_;
};

TestHelpers::Timer::Timer()
{
    reset();
}

void TestHelpers::Timer::reset()
{
    timeBeg_ = 0;
    timeSum_ = 0;
    timeMin_ = 0;
    timeMax_ = 0;
    numOfPasses_ = 0;
}

void TestHelpers::Timer::beginPass()
{
    timeBeg_ = juce::Time::getMillisecondCounterHiRes();
}

void TestHelpers::Timer::endPass()
{
    double timePass = juce::Time::getMillisecondCounterHiRes() - timeBeg_;
    
    if (0 == numOfPasses_)
    {
        timeSum_ = timePass;
        timeMin_ = timePass;
        timeMax_ = timePass;
    }
    else
    {
        timeSum_ += timePass;
        timeMin_ = timePass < timeMin_ ? timePass : timeMin_;
        timeMax_ = timePass > timeMax_ ? timePass : timeMax_;
    }
    
    ++numOfPasses_;
}

void TestHelpers::log (const std::string& msg)
{
    ((ThisApplication*)JUCEApplication::getInstance())->mainView().log (false, msg);
    std::cout << msg;
}

class TestHelpers::Thread : public juce::Thread
{
public:
    std::function<void (void)>  f;
    
    Thread(const char* name) : juce::Thread (name)
    {
        
    }
    
    void run() override
    {
        f();
    }
};

long TestHelpers::startThread (std::function<void (void)> func, const char* name)
{
    TestHelpers::Thread* thread = new TestHelpers::Thread (name);
    thread->f = func;
    thread->startThread();
    
    return reinterpret_cast<long> (thread);
}

void TestHelpers::waitForThreadExit(long threadHandle)
{
    TestHelpers::Thread* thread = reinterpret_cast<TestHelpers::Thread*>(threadHandle);
    thread->stopThread(-1);
    
    delete thread;
}

void TestHelpers::sleep (int milliseconds)
{
    Thread::sleep (milliseconds);
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (ThisApplication)

#endif
