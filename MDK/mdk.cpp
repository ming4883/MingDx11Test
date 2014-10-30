#include "mdk.h"

#include "mdk_Allocator.cpp"
#include "mdk_BabylonFile.cpp"
#include "mdk_Demo.cpp"
#include "mdk_Math.cpp"
#include "mdk_Persistable.cpp"
#include "mdk_Scene.cpp"

#if JUCE_WINDOWS
#include "mdk_D3D11.cpp"
#include "mdk_D3D11Scene.cpp"
#endif // JUCE_WINDOWS

#ifdef MDK_UNIT_TESTS
#include "mdk_Scene.tests.cpp"
#include "mdk_SOAManager.tests.cpp"
#endif
