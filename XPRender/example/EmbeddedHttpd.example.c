#include "Common.h"
#include "Remote.h"

RemoteConfig* _config = nullptr;
float bgR = 255;
float bgG = 127;
float bgB = 0;

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
}

void xprAppHandleMouse(int x, int y, int action)
{
}

void xprAppRender()
{
	float r, g, b;
	
	remoteConfigLock(_config);
	r = bgR / 255; g = bgG / 255; b = bgB / 255;
	remoteConfigUnlock(_config);

	xprRenderTargetClearColor(r, g, b, 1.0f);
}

void xprAppConfig()
{
	xprAppContext.appName = "Embedded Httpd";
	xprAppContext.xres = 480;
	xprAppContext.yres = 320;
	xprAppContext.multiSampling = XprFalse;
	xprAppContext.vsync = XprFalse;
	xprAppContext.apiMajorVer = 2;
	xprAppContext.apiMinorVer = 1;
}

void xprAppFinalize()
{
	remoteConfigFree(_config);
}

XprBool xprAppInitialize()
{
	RemoteVarDesc descs[] = {
		{"bgR", &bgR, 0, 255},
		{"bgG", &bgG, 0, 255},
		{"bgB", &bgB, 0, 255},
		{nullptr, nullptr, 0, 0}
	};

	xprRenderTargetSetViewport(0, 0, (float)xprAppContext.xres, (float)xprAppContext.yres, -1, 1);

	_config = remoteConfigAlloc();
	remoteConfigInit(_config, 80, XprTrue);
	remoteConfigAddVars(_config, descs);

	return XprTrue;
}
