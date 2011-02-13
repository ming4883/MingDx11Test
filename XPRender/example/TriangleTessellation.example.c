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
	
	xprGpuStateSetDepthTestEnabled(_gpuState, XprFalse);
	xprGpuStateSetCullEnabled(_gpuState, XprTrue);
	xprGpuStatePreRender(_gpuState);

	xprGpuProgramPreRender(_bgMaterial->program);
	xprGpuProgramUniform4fv(_bgMaterial->program, XprHash("u_colors"), 4, (const float*)c);

	Mesh_preRender(_bgMesh, _bgMaterial->program);
	Mesh_render(_bgMesh);
}

void drawScene(Settings* settings)
{
	XprVec3 eyeAt = xprVec3(-2.5f, 1.5f, 5);
	XprVec3 lookAt = xprVec3(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;
	XprMat44 projMtx;
	XprMat44 viewProjMtx;
	
	xprMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44Prespective(&projMtx, 45.0f, _aspect.width / _aspect.height, 0.1f, 30.0f);
	xprMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	xprGpuStateSetDepthTestEnabled(_gpuState, XprTrue);
	xprGpuStateSetCullEnabled(_gpuState, XprTrue);
	//xprGpuStateSetPolygonMode(_gpuState, XprGpuState_PolygonMode_Line);
	xprGpuStatePreRender(_gpuState);

	xprGpuProgramPreRender(_sceneMaterial->program);
	
	// draw floor
	_renderContext.matDiffuse = xprVec4(1.0f, 0.88f, 0.33f, 1);
	_renderContext.matSpecular = xprVec4(2, 2, 2, 1);
	_renderContext.matShininess = 32;

	{
		XprMat44 m;
		XprVec3 axis = {1, 0, 0};
		xprMat44MakeRotation(&m, &axis, -90);
		
		xprMat44Mult(&_renderContext.worldViewMtx, &viewMtx, &m);
		xprMat44Mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
	}

	RenderContext_preRender(&_renderContext, _sceneMaterial);

	xprGpuProgramUniform1fv(_sceneMaterial->program, XprHash("u_tessLevel"), 1, (const float*)&(settings->tessLevel));
	xprGpuProgramUniform1fv(_sceneMaterial->program, XprHash("u_linearity"), 1, (const float*)&(settings->linearity));

	Mesh_preRender(_tessMesh, _sceneMaterial->program);
	Mesh_renderPatches(_tessMesh, _tessMesh->vertexPerPatch);

	xprGpuStateSetPolygonMode(_gpuState, XprGpuState_PolygonMode_Fill);
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

	xprRenderTargetClearDepth(1);

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
	xprGpuStateFree(_gpuState);
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

	_gpuState = xprGpuStateAlloc();
	xprGpuStateInit(_gpuState);

	xprRenderTargetSetViewport(0, 0, (float)width, (float)height, -1, 1);
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
