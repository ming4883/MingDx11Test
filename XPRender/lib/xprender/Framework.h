#ifndef __XPRENDER_FRAMEWORK_H__
#define __XPRENDER_FRAMEWORK_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// Xpr is a platform abstraction layer for rendering;
// To create a cross-platform app, simply link against the Xpr source and implement the following functions.
//
void xprAppConfig();								// application configuration. i.e. fill in xprAppContext
XprBool xprAppInitialize();							// application initialization
void xprAppRender();								// draw scene (Xpr swaps the backbuffer for you)
void xprAppUpdate(unsigned int milliseconds);		// receive elapsed time (e.g., update physics)
void xprAppHandleMouse(int x, int y, int action);	// handle mouse action: PEZ_DOWN, PEZ_UP, or PEZ_MOVE
void xprAppFinalize();								// application finalization

typedef struct XprAppContext
{
	const char* appName;
	int apiMajorVer;
	int apiMinorVer;
	XprBool multiSampling;
	XprBool vsync;
	int xres;
	int yres;

} XprAppContext;

enum
{
	XprApp_MouseDown,
	XprApp_MouseUp,
	XprApp_MouseMove,
};

extern XprAppContext xprAppContext;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_FRAMEWORK_H__