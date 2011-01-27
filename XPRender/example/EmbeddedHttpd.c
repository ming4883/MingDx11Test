#include "Common.h"

void PezUpdate(unsigned int elapsedMilliseconds)
{
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	glClearDepth(1);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	{ // check for any OpenGL errors
	GLenum glerr = glGetError();

	if(glerr != GL_NO_ERROR)
		PezDebugString("GL has error %4x!", glerr);
	}
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 640;
	PEZ_VIEWPORT_HEIGHT = 480;
	PEZ_ENABLE_MULTISAMPLING = 0;
	PEZ_VERTICAL_SYNC = 0;
}

void PezExit(void)
{
}

const char* PezInitialize(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	
	atexit(PezExit);

	return "Embedded Httpd";
}
