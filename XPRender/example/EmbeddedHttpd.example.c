#include "Common.h"
#include "Remote.h"

RemoteConfig* _config = nullptr;
float bgR = 255;
float bgG = 127;
float bgB = 0;

void PezUpdate(unsigned int elapsedMilliseconds)
{
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	float r, g, b;
	
	RemoteConfig_lock(_config);
	r = bgR / 255; g = bgG / 255; b = bgB / 255;
	RemoteConfig_unlock(_config);

	xprRenderTargetClearColor(r, g, b, 1.0f);
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 480;
	PEZ_VIEWPORT_HEIGHT = 320;
	PEZ_ENABLE_MULTISAMPLING = 0;
	PEZ_VERTICAL_SYNC = 0;
}

void PezFinalize()
{
	RemoteConfig_free(_config);
}

const char* PezInitialize(int width, int height)
{
	RemoteVarDesc descs[] = {
		{"bgR", &bgR, 0, 255},
		{"bgG", &bgG, 0, 255},
		{"bgB", &bgB, 0, 255},
		{nullptr, nullptr, 0, 0}
	};

	xprRenderTargetSetViewport(0, 0, (float)width, (float)height, -1, 1);

	_config = RemoteConfig_alloc();
	RemoteConfig_init(_config, 80, XprTrue);
	RemoteConfig_addVars(_config, descs);

	return "Embedded Httpd";
}
