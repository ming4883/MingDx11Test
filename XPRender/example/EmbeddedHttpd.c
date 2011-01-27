#include "Common.h"
#include "../lib/httpd/httpd.h"
#include <winsock2.h>

httpd* _httpd = nullptr;

void PezUpdate(unsigned int elapsedMilliseconds)
{
	struct timeval to;
	to.tv_sec = 0;
	to.tv_usec = 1000;
	
	if (httpdGetConnection(_httpd, &to) <= 0)
		return;

	if(httpdReadRequest(_httpd) < 0) {
		httpdEndRequest(_httpd);
		return;
	}

	httpdProcessRequest(_httpd);

	httpdEndRequest(_httpd);
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
	httpdDestroy(_httpd);
}

void index_html(httpd* server)
{
	httpdPrintf(server, "hello HTTPD");
}

const char* PezInitialize(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	_httpd = httpdCreate(nullptr, HTTP_PORT);
	httpdAddCContent(_httpd, "/", "index.html", HTTP_TRUE, nullptr, index_html);
	
	atexit(PezExit);

	return "Embedded Httpd";
}
