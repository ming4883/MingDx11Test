#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Remote.h"

XprGpuState* _gpuState = nullptr;
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

typedef struct Settings
{
	float tessLevel;
	float linearity;
} Settings;

Settings _settings;

RemoteConfig* _config = nullptr;

void drawBackground()
{
	static const XprVec4 c[] = {
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.145f, 0.31f, 0.405f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
	};
	
	XprGpuState_setDepthTestEnabled(_gpuState, XprFalse);
	XprGpuState_setCullEnabled(_gpuState, XprTrue);
	XprGpuState_preRender(_gpuState);

	XprGpuProgram_preRender(_bgMaterial->program);
	XprGpuProgram_uniform4fv(_bgMaterial->program, XprHash("u_colors"), 4, (const float*)c);

	Mesh_preRender(_bgMesh, _bgMaterial->program);
	Mesh_render(_bgMesh);
}

void drawScene(Settings* settings)
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

	XprGpuState_setDepthTestEnabled(_gpuState, XprTrue);
	XprGpuState_setCullEnabled(_gpuState, XprTrue);
	//XprGpuState_setPolygonMode(_gpuState, XprGpuState_PolygonMode_Line);
	XprGpuState_preRender(_gpuState);

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
		RenderContext_preRender(&_renderContext, _sceneMaterial);

		XprGpuProgram_uniform1fv(_sceneMaterial->program, XprHash("u_tessLevel"), 1, (const float*)&(settings->tessLevel));
		XprGpuProgram_uniform1fv(_sceneMaterial->program, XprHash("u_linearity"), 1, (const float*)&(settings->linearity));

		Mesh_preRender(_tessMesh, _sceneMaterial->program);

		
		Mesh_renderPatches(_tessMesh, _tessMesh->vertexPerPatch);
	}

	XprGpuState_setPolygonMode(_gpuState, XprGpuState_PolygonMode_Fill);
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
	Settings settings;

	RemoteConfig_lock(_config);
	settings = _settings;
	settings.linearity /= 10.0f;
	RemoteConfig_unlock(_config);

	XprRenderTarget_clearDepth(1);

	drawBackground();
	drawScene(&settings);
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
	XprGpuState_free(_gpuState);
	RemoteConfig_free(_config);
}

const char* PezInitialize(int width, int height)
{
	RemoteVarDesc descs[] = {
		{"tessLevel", &_settings.tessLevel, 0, 16},
		{"linearity", &_settings.linearity, 0, 10},
		{nullptr, nullptr, 0, 0},
	};

	_settings.tessLevel = 8;
	_settings.linearity = 3;

	_config = RemoteConfig_alloc();
	RemoteConfig_init(_config, 80, XprTrue);
	RemoteConfig_addVars(_config, descs);

	_gpuState = XprGpuState_alloc();
	XprGpuState_init(_gpuState);

	XprRenderTarget_setViewport(0, 0, (float)width, (float)height, -1, 1);
	_aspect.width = (float)width;
	_aspect.height = (float)height;

	_mouse.isDown = XprFalse;

	// materials
	glswInit(&myFileSystem);
	glswSetPath("", ".glsl");
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

	Mesh_initWithObjFile(_tessMesh, "monkey.obj", &myInputStream);

	_bgMesh = Mesh_alloc();
	Mesh_initWithScreenQuad(_bgMesh);
	
	atexit(PezExit);

	return "Triangle Tessellation";
}
