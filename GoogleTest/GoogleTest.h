#ifndef JUCE_GTEST_H_INCLUDED
#define JUCE_GTEST_H_INCLUDED

// Make sure /I"..\..\GoogleTest\include" was added to the Extra compiler flags in the .jucer file.
// Define JUCE_GOOGLE_TEST_NAME for the name of the test window.
#include <gtest/gtest.h>
#include <functional>

class TestHelpers
{
public:
    class Timer
    {
    private:
        double timeBeg_;
        double timeSum_;
        double timeMin_;
        double timeMax_;
        int numOfPasses_;
        
    public:
        Timer();
        void reset();
        void beginPass();
        void endPass();
        
        int numOfPasses() const { return numOfPasses_; }
        double sum() const { return timeSum_; }
        double min() const { return timeMin_; }
        double max() const { return timeMax_; }
        double average() const { if (0 == numOfPasses_) return 0; return timeSum_ / numOfPasses_; }
    };
    
    static void log (const std::string& msg);
    
    class Thread;
    static long startThread (std::function<void (void)> func, const char* name = "TestHelpers");
    static void waitForThreadExit (long threadHandle);
    static void sleep (int numofMS);
    
    template<class T>
    static inline void doNotOptimizeAway (T& x)
    {
        printf ("%p", &x);
    }
};


#endif  // JUCE_GTEST_H_INCLUDED
