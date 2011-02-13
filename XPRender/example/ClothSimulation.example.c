#include "Common.h"
#include "Remote.h"
#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Material.h"
#include "Pvr.h"
#include "red_tile_texture.h"

#include "../lib/xprender/Texture.h"
#include "../lib/xprender/RenderTarget.h"

RemoteConfig* _config = nullptr;
Cloth* _cloth = nullptr;
#define BallCount 2
Sphere _ball[BallCount];
Mesh* _ballMesh = nullptr;
Mesh* _floorMesh = nullptr;
Mesh* _bgMesh = nullptr;
Material* _sceneMaterial = nullptr;
Material* _bgMaterial = nullptr;
Material* _uiMaterial = nullptr;
Material* _textMaterial = nullptr;
XprTexture* _texture = nullptr;
XprRenderTarget* _rt = nullptr;
XprGpuState* _gpuState = nullptr;
XprVec3 _floorN = {0, 1, 0};
XprVec3 _floorP = {0, 0, 0};

RenderContext _renderContext;

typedef struct Settings
{
	float gravity;
	float airResistance;
	float impact;
} Settings;

Settings _settings;

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
	XprVec3 clothOffsets[2];
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
	
	xprGpuStateSetDepthTestEnabled(_gpuState, XprFalse);
	xprGpuStateSetCullEnabled(_gpuState, XprTrue);
	xprGpuStatePreRender(_gpuState);

	xprGpuProgramPreRender(_bgMaterial->program);
	xprGpuProgramUniform4fv(_bgMaterial->program, XprHash("u_colors"), 4, (const float*)c);

	Mesh_preRender(_bgMesh, _bgMaterial->program);
	Mesh_render(_bgMesh);
}

void drawScene()
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

	xprGpuStateSetCullEnabled(_gpuState, XprTrue);
	xprGpuStateSetDepthTestEnabled(_gpuState, XprTrue);
	xprGpuStatePreRender(_gpuState);

	xprGpuProgramPreRender(_sceneMaterial->program);
	xprGpuProgramUniformTexture(_sceneMaterial->program, XprHash("u_tex"), _texture);

	{	// draw floor
		_renderContext.matDiffuse = xprVec4(1.0f, 0.88f, 0.33f, 1);
		_renderContext.matSpecular = xprVec4(0, 0, 0, 1);
		_renderContext.matShininess = 32;
		{
			XprMat44 m;
			XprVec3 axis = {1, 0, 0};
			xprMat44MakeRotation(&m, &axis, -90);
			
			xprMat44Mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		RenderContext_preRender(&_renderContext, _sceneMaterial);

		Mesh_preRender(_floorMesh, _sceneMaterial->program);
		Mesh_render(_floorMesh);
	}

	{	// draw cloth
		_renderContext.matDiffuse = xprVec4(1.0f, 0.22f, 0.0f, 1);
		_renderContext.matSpecular = xprVec4(0.125f, 0.125f, 0.125f, 1);
		_renderContext.matShininess = 32;

		xprGpuStateSetCullEnabled(_gpuState, XprFalse);
		xprGpuStatePreRender(_gpuState);
		{
			XprMat44 m;
			xprMat44SetIdentity(&m);

			xprMat44Mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		RenderContext_preRender(&_renderContext, _sceneMaterial);

		Mesh_preRender(_cloth->mesh, _sceneMaterial->program);
		Mesh_render(_cloth->mesh);
		
		xprGpuStateSetCullEnabled(_gpuState, XprTrue);
		xprGpuStatePreRender(_gpuState);
	}

	{	// draw ball
		int i;
		_renderContext.matDiffuse = xprVec4(0.9f, 0.64f, 0.35f, 1);
		_renderContext.matSpecular = xprVec4(1, 1, 1, 1);
		_renderContext.matShininess = 32;

		for(i=0; i<BallCount; ++i) {
			XprMat44 m;
			XprVec3 scale = {_ball[i].radius, _ball[i].radius, _ball[i].radius};
			xprMat44MakeScale(&m, &scale);
			xprMat44SetTranslation(&m, &_ball[i].center);
			
			xprMat44Mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);

			RenderContext_preRender(&_renderContext, _sceneMaterial);
			
			Mesh_preRender(_ballMesh, _sceneMaterial->program);
			Mesh_render(_ballMesh);
		}
	}
}

void PezUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings settings;

	int iter;
	XprVec3 f;

	RemoteConfig_lock(_config);
	settings = _settings;
	RemoteConfig_unlock(_config);

	t += 0.0005f * _settings.impact;

	_ball[0].center.z = cosf(t) * 5.f;
	_ball[1].center.z = sinf(t) * 5.f;

	_cloth->timeStep = 0.01f;	// fixed time step
	_cloth->damping = _settings.airResistance * 1e-3f;

	// perform relaxation
	for(iter = 0; iter < 5; ++iter)
	{
		int i;
		for(i=0; i<BallCount; ++i)
			Cloth_collideWithSphere(_cloth, &_ball[i]);

		Cloth_collideWithPlane(_cloth, &_floorN, &_floorP);
		Cloth_satisfyConstraints(_cloth);
	}
	
	f = xprVec3(0, -_settings.gravity * _cloth->timeStep, 0);
	Cloth_addForceToAll(_cloth, &f);

	Cloth_verletIntegration(_cloth);

	Cloth_updateMesh(_cloth);
}

void PezHandleMouse(int x, int y, int action)
{
	if(PEZ_DOWN == action) {
		_mouse.x = x;
		_mouse.y = y;

		_mouse.clothOffsets[0] = _cloth->fixPos[0];
		_mouse.clothOffsets[1] = _cloth->fixPos[_cloth->segmentCount-1];

		_mouse.isDown = XprTrue;
	}
	else if(PEZ_UP == action) {
		_mouse.isDown = XprFalse;
	}
	else if((PEZ_MOVE == action) && (XprTrue == _mouse.isDown)) {
		int dx = x - _mouse.x;
		int dy = y - _mouse.y;
		
		float mouseSensitivity = 0.0025f;
		_cloth->fixPos[0].x = _mouse.clothOffsets[0].x + dx * mouseSensitivity;
		_cloth->fixPos[_cloth->segmentCount-1].x = _mouse.clothOffsets[1].x + dx * mouseSensitivity;

		_cloth->fixPos[0].y = _mouse.clothOffsets[0].y + dy * -mouseSensitivity;
		_cloth->fixPos[_cloth->segmentCount-1].y = _mouse.clothOffsets[1].y + dy * -mouseSensitivity;
	}
}

void PezRender()
{
	// render to texture
	XprRenderBufferHandle color = xprRenderTargetAcquireBuffer(_rt, "unormR8G8B8A8");
	XprRenderBufferHandle depth = xprRenderTargetAcquireBuffer(_rt, "depth16");
	XprTexture* tex = XprRenderTarget_getTexture(_rt, color);

	XprRenderBufferHandle bufs[] = {color, nullptr};
	xprRenderTargetPreRender(_rt, bufs, depth);

	xprRenderTargetClearDepth(1);

	drawBackground();
	drawScene();

	// display the rendered image in color buffer
	xprRenderTargetPreRender(nullptr, nullptr, nullptr);

	xprGpuStateSetDepthTestEnabled(_gpuState, XprFalse);
	xprGpuStateSetCullEnabled(_gpuState, XprTrue);
	xprGpuStatePreRender(_gpuState);

	xprGpuProgramPreRender(_uiMaterial->program);
	xprGpuProgramUniformTexture(_uiMaterial->program, XprHash("u_tex"), tex);
	
	Mesh_preRender(_bgMesh, _uiMaterial->program);
	Mesh_render(_bgMesh);
	
	xprRenderTargetReleaseBuffer(_rt, color);
	xprRenderTargetReleaseBuffer(_rt, depth);
}

void PezConfig()
{
	PEZ_VIEWPORT_WIDTH = 800;
	PEZ_VIEWPORT_HEIGHT = 600;
	PEZ_ENABLE_MULTISAMPLING = 1;
	PEZ_VERTICAL_SYNC = 0;
	PEZ_GL_VERSION_MAJOR = 3;
	PEZ_GL_VERSION_MINOR = 3;
}

void PezExit(void)
{
	RemoteConfig_free(_config);
	Cloth_free(_cloth);
	Mesh_free(_ballMesh);
	Mesh_free(_floorMesh);
	Mesh_free(_bgMesh);
	Material_free(_sceneMaterial);
	Material_free(_bgMaterial);
	Material_free(_uiMaterial);
	Material_free(_textMaterial);
	xprTextureFree(_texture);
	xprRenderTargetFree(_rt);
	xprGpuStateFree(_gpuState);
}

const char* PezInitialize(int width, int height)
{
	// remote config
	RemoteVarDesc descs[] = {
		{"gravity", &_settings.gravity, 1, 100},
		{"airResistance", &_settings.airResistance, 1, 20},
		{"impact", &_settings.impact, 1, 10},
		{nullptr, nullptr, 0, 0}
	};

	_settings.gravity = 10;
	_settings.airResistance = 5;
	_settings.impact = 3;

	_config = RemoteConfig_alloc();
	RemoteConfig_init(_config, 80, XprTrue);
	RemoteConfig_addVars(_config, descs);

	xprRenderTargetSetViewport(0, 0, (float)width, (float)height, -1, 1);
	_aspect.width = (float)width;
	_aspect.height = (float)height;

	_mouse.isDown = XprFalse;

	_gpuState = xprGpuStateAlloc();
	xprGpuStateInit(_gpuState);

	_rt = xprRenderTargetAlloc();
	xprRenderTargetInit(_rt, (size_t)width, (size_t)height);

	// materials
	glswInit(&myFileSystem);
	glswSetPath("", ".glsl");
	glswAddDirectiveToken("","#version 150");

	_sceneMaterial = loadMaterial(
		"ClothSimulation.Scene.Vertex",
		"ClothSimulation.Scene.Fragment",
		nullptr, nullptr, nullptr);
	
	_bgMaterial = loadMaterial(
		"Common.Bg.Vertex",
		"Common.Bg.Fragment",
		nullptr, nullptr, nullptr);
	
	_uiMaterial = loadMaterial(
		"Common.Ui.Vertex",
		"Common.Ui.Fragment",
		nullptr, nullptr, nullptr);
	
	_textMaterial = loadMaterial(
		"Common.Ui.Vertex",
		"Common.Text.Fragment",
		nullptr, nullptr, nullptr);

	_texture = Pvr_createTexture(red_tile_texture);
	
	glswShutdown();

	{
		XprVec3 offset = xprVec3(-1, 1.5f, 0);
		_cloth = Cloth_new(2, 2, &offset, 32);
	}

	_ball[0].center = xprVec3(-0.5f, 0.5f, 0);
	_ball[0].radius = 0.25f;
	_ball[1].center = xprVec3(0.5f, 0.5f, 0);
	_ball[1].radius = 0.25f;
	_ballMesh = Mesh_alloc();
	Mesh_initWithUnitSphere(_ballMesh, 32);

	{
		XprVec3 offset = xprVec3(-2.5f, -2.5f, 0);
		_floorMesh = Mesh_alloc();
		Mesh_initWithQuad(_floorMesh, 5, 5, &offset, 1);
	}

	_bgMesh = Mesh_alloc();
	Mesh_initWithScreenQuad(_bgMesh);
	
	atexit(PezExit);

	return "Cloth Simulation";
}
