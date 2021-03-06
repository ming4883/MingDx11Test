#include "mdk.h"

#include "mdk_Allocator.cpp"
#include "mdk_BabylonFile.cpp"
#include "mdk_Demo.cpp"
#include "mdk_Frontend.cpp"
#include "mdk_Graphics.cpp"
#include "mdk_Math.cpp"
#include "mdk_Persistable.cpp"
#include "mdk_Scene.cpp"

#if JUCE_WINDOWS
#include "mdk_D3D11.cpp"
#include "mdk_D3D11Scene.cpp"
#endif // JUCE_WINDOWS

#ifdef MDK_UNIT_TESTS
#define MDK_TEST_GRAPHICS 1
#include "mdk_Frontend.tests.cpp"
#include "mdk_Graphics.tests.cpp"
#include "mdk_Scene.tests.cpp"
#include "mdk_SOAManager.tests.cpp"
#endif
