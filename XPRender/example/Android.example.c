#include <pez.h>

#include <GLES/gl.h>

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
	glClearColor(1, 0.5f, 0.5f, 1);
	glClear(GL_COLOR_BUFFER_BIT);
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
