#include <pez.h>
#include "../lib/xprender/RenderTarget.h"

void PezUpdate(unsigned int elapsedMilliseconds)
{
	PezDebugString("PezUpdate");
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	PezDebugString("PezRender");
	XprRenderTarget_clearColor(1, 0.25f, 0.25f, 1);
}

void PezConfig()
{
	PezDebugString("PezConfig");
}

void PezExit(void)
{
}

const char* PezInitialize(int width, int height)
{
	return "Android Example";
}
