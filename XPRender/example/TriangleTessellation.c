#include "Common.h"
#include "Mesh.h"
#include "Material.h"

Mesh* _tessMesh = nullptr;
Mesh* _bgMesh = nullptr;
Material* _sceneMaterial = nullptr;
Material* _bgMaterial = nullptr;

RenderContext _renderContext;

typedef struct Aspect
{
	float width;
	float height;
} Aspect;

Aspect _aspect;

typedef struct Mouse
{
	int x;
	int y;
	XprBool isDown;
} Mouse;

Mouse _mouse;


void drawBackground()
{
	static const XprVec4 c[] = {
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.145f, 0.31f, 0.405f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
	};
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	XprGpuProgram_preRender(_bgMaterial->program);
	XprGpuProgram_uniform4fv(_bgMaterial->program, XPR_HASH("u_colors"), 4, (const float*)c);

	Mesh_preRender(_bgMesh, _bgMaterial->program);
	Mesh_render(_bgMesh);
}

void drawScene()
{
	XprVec3 eyeAt = XprVec3_(-2.5f, 1.5f, 5);
	XprVec3 lookAt = XprVec3_(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;
	XprMat44 projMtx;
	XprMat44 viewProjMtx;
	
	XprMat44_cameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	XprMat44_prespective(&projMtx, 45.0f, _aspect.width / _aspect.height, 0.1f, 30.0f);
	XprMat44_mult(&viewProjMtx, &projMtx, &viewMtx);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	XprGpuProgram_preRender(_sceneMaterial->program);
	
	{	// draw floor
		_renderContext.matDiffuse = XprVec4_(1.0f, 0.88f, 0.33f, 1);
		_renderContext.matSpecular = XprVec4_(2, 2, 2, 1);
		_renderContext.matShininess = 32;
		{
			XprMat44 m;
			XprVec3 axis = {1, 0, 0};
			XprMat44_makeRotation(&m, &axis, -90);
			
			XprMat44_mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			XprMat44_mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		RenderContext_apply(&_renderContext, _sceneMaterial);

		Mesh_preRender(_tessMesh, _sceneMaterial->program);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Mesh_renderPatches(_tessMesh, 3);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void PezUpdate(unsigned int elapsedMilliseconds)
{
}

void PezHandleMouse(int x, int y, int action)
{
	if(PEZ_DOWN == action) {
		_mouse.x = x;
		_mouse.y = y;

		_mouse.isDown = XprTrue;
	}
	else if(PEZ_UP == action) {
		_mouse.isDown = XprFalse;
	}
	else if((PEZ_MOVE == action) && (XprTrue == _mouse.isDown)) {
		int dx = x - _mouse.x;
		int dy = y - _mouse.y;
		
		float mouseSensitivity = 0.0025f;
	}
}

void PezRender()
{
	glClearDepth(1);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawBackground();
	drawScene();
	
	{ // check for any OpenGL errors
	GLenum glerr = glGetError();

	if(glerr != GL_NO_ERROR)
		PezDebugString("GL has error %4x!", glerr);
	}
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 800;
	PEZ_VIEWPORT_HEIGHT = 600;
	PEZ_ENABLE_MULTISAMPLING = 0;
	PEZ_VERTICAL_SYNC = 0;
	PEZ_GL_VERSION_MAJOR = 4;
	PEZ_GL_VERSION_MINOR = 0;
}

void PezExit(void)
{
	Mesh_free(_tessMesh);
	Mesh_free(_bgMesh);
	Material_free(_sceneMaterial);
	Material_free(_bgMaterial);
}

const char* PezInitialize(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	_aspect.width = (float)width;
	_aspect.height = (float)height;

	_mouse.isDown = XprFalse;

	// materials
	glswInit();
	glswSetPath("../example/", ".glsl");
	glswAddDirectiveToken("","#version 400");

	_sceneMaterial = loadMaterial(
		"TriangleTessellation.Scene.Vertex",
		"TriangleTessellation.Scene.Fragment",
		"TriangleTessellation.Scene.TessControl",
		"TriangleTessellation.Scene.TessEvaluation",
		nullptr);
	
	_bgMaterial = loadMaterial(
		"Common.Bg.Vertex",
		"Common.Bg.Fragment",
		nullptr, nullptr, nullptr);
	
	glswShutdown();

	_tessMesh = Mesh_alloc();
	Mesh_initWithUnitSphere(_tessMesh, 6);

	_bgMesh = Mesh_alloc();
	Mesh_initWithScreenQuad(_bgMesh);
	
	atexit(PezExit);

	return "Triangle Tessellation";
}
