#include "Common.h"
#include "Remote.h"

RemoteConfig* _config = nullptr;
float bgR = 255;
float bgG = 127;
float bgB = 0;

void PezUpdate(unsigned int elapsedMilliseconds)
{
	RemoteConfig_processRequest(_config);
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	glClearDepth(1);
	glClearColor(bgR / 255, bgG / 255, bgB / 255, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 480;
	PEZ_VIEWPORT_HEIGHT = 320;
	PEZ_ENABLE_MULTISAMPLING = 0;
	PEZ_VERTICAL_SYNC = 0;
}

void PezExit(void)
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

	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	_config = RemoteConfig_alloc();
	RemoteConfig_init(_config, 80);
	RemoteConfig_addVars(_config, descs);

	atexit(PezExit);

	return "Embedded Httpd";
}
