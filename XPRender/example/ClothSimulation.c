#include <GL/glew.h>

#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"
#include "../lib/xprender/Mat44.h"
#include "../lib/xprender/Shader.h"
#include "../lib/xprender/Texture.h"
#include "../lib/xprender/RenderTarget.h"
#include "../lib/glsw/glsw.h"
#include "../lib/pez/pez.h"

#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Material.h"
#include "Label.h"
#include "Pvr.h"
#include "red_tile_texture.h"

Cloth* _cloth = nullptr;
#define BallCount 2
Sphere _ball[BallCount];
Mesh* _ballMesh = nullptr;
Mesh* _floorMesh = nullptr;
Mesh* _bgMesh = nullptr;
Material* _sceneMaterial = nullptr;
Material* _bgMaterial = nullptr;
Material* _uiMaterial = nullptr;
XprTexture* _texture = nullptr;
XprRenderTarget* _rt = nullptr;
XprVec3 _floorN = {0, 1, 0};
XprVec3 _floorP = {0, 0, 0};
float _gravity = 10;
float _airResistance = 5;
float _impact = 3;
XprBool _showDebug = XprFalse;
Label* _label = nullptr;

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

typedef struct RenderContext
{
	XprMat44 worldViewProjMtx;
	XprMat44 worldViewMtx;
	XprVec4 matDiffuse;
	XprVec4 matSpecular;
	float matShininess;

} RenderContext;

void RenderContext_apply(RenderContext* self, Material* material)
{
	XprGpuProgram_uniformMtx4fv(material->program, XPR_HASH("u_worldViewMtx"), 1, XprTrue, self->worldViewMtx.v);
	XprGpuProgram_uniformMtx4fv(material->program, XPR_HASH("u_worldViewProjMtx"), 1, XprTrue, self->worldViewProjMtx.v);
	XprGpuProgram_uniform4fv(material->program, XPR_HASH("u_matDiffuse"), 1, self->matDiffuse.v);
	XprGpuProgram_uniform4fv(material->program, XPR_HASH("u_matSpecular"), 1, self->matSpecular.v);
	XprGpuProgram_uniform1fv(material->program, XPR_HASH("u_matShininess"), 1, &self->matShininess);
}

RenderContext _renderContext;

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
	XprGpuProgram_uniformTexture(_sceneMaterial->program, XPR_HASH("u_tex"), _texture);

	{	// draw floor
		_renderContext.matDiffuse = XprVec4_(1.0f, 0.88f, 0.33f, 1);
		_renderContext.matSpecular = XprVec4_(0, 0, 0, 1);
		_renderContext.matShininess = 32;
		{
			XprMat44 m;
			XprVec3 axis = {1, 0, 0};
			XprMat44_makeRotation(&m, &axis, -90);
			
			XprMat44_mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			XprMat44_mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		RenderContext_apply(&_renderContext, _sceneMaterial);

		Mesh_preRender(_floorMesh, _sceneMaterial->program);
		Mesh_render(_floorMesh);
	}

	{	// draw cloth
		_renderContext.matDiffuse = XprVec4_(1.0f, 0.22f, 0.0f, 1);
		_renderContext.matSpecular = XprVec4_(0.125f, 0.125f, 0.125f, 1);
		_renderContext.matShininess = 32;

		glDisable(GL_CULL_FACE);
		{
			XprMat44 m;
			XprMat44_setIdentity(&m);

			XprMat44_mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			XprMat44_mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		RenderContext_apply(&_renderContext, _sceneMaterial);

		Mesh_preRender(_cloth->mesh, _sceneMaterial->program);
		Mesh_render(_cloth->mesh);
		
		glEnable(GL_CULL_FACE);
	}

	{	// draw ball
		int i;
		_renderContext.matDiffuse = XprVec4_(0.9f, 0.64f, 0.35f, 1);
		_renderContext.matSpecular = XprVec4_(1, 1, 1, 1);
		_renderContext.matShininess = 32;

		for(i=0; i<BallCount; ++i) {
			XprMat44 m;
			XprVec3 scale = {_ball[i].radius, _ball[i].radius, _ball[i].radius};
			XprMat44_makeScale(&m, &scale);
			XprMat44_setTranslation(&m, &_ball[i].center);
			
			XprMat44_mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			XprMat44_mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);

			RenderContext_apply(&_renderContext, _sceneMaterial);

			Mesh_preRender(_ballMesh, _sceneMaterial->program);
			Mesh_render(_ballMesh);
		}
	}
}

Material* loadMaterial(const char* vsKey, const char* fsKey)
{
	const char* args[] = {
		"vs", glswGetShader(vsKey),
		"fs", glswGetShader(fsKey),
		nullptr,
	};
	
	Material* material = Material_alloc();
	Material_initWithShaders(material, args);

	if(0 == (material->flags & MaterialFlag_Inited))
		PezDebugString("failed to load material vs=%s,fs=%s!\n", vsKey, fsKey);

	return material;
}

void PezUpdate(unsigned int elapsedMilliseconds)
{
	int iter;
	XprVec3 f;

	static float t = 0;
	t += 0.0005f * _impact;
	
	_ball[0].center.z = cosf(t) * 5.f;
	_ball[1].center.z = sinf(t) * 5.f;

	_cloth->timeStep = 0.01f;	// fixed time step
	_cloth->damping = _airResistance * 1e-3f;

	// perform relaxation
	for(iter = 0; iter < 5; ++iter)
	{
		int i;
		for(i=0; i<BallCount; ++i)
			Cloth_collideWithSphere(_cloth, &_ball[i]);

		Cloth_collideWithPlane(_cloth, &_floorN, &_floorP);
		Cloth_satisfyConstraints(_cloth);
	}
	
	f = XprVec3_(0, -_gravity * _cloth->timeStep, 0);
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
	XprRenderBufferHandle color = XprRenderTarget_acquireBuffer(_rt, "unormR8G8B8A8");
	XprRenderBufferHandle depth = XprRenderTarget_acquireBuffer(_rt, "depth16");
	XprTexture* tex = XprRenderTarget_getTexture(_rt, color);

	XprRenderBufferHandle bufs[] = {color, nullptr};
	XprRenderTarget_preRender(_rt, bufs, depth);

	glClearDepth(1);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawBackground();
	drawScene();

	// display the rendered image in color buffer
	XprRenderTarget_preRender(nullptr, nullptr, nullptr);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	XprGpuProgram_preRender(_uiMaterial->program);
	XprGpuProgram_uniformTexture(_uiMaterial->program, XPR_HASH("u_tex"), tex);
	
	Mesh_preRender(_bgMesh, _uiMaterial->program);
	Mesh_render(_bgMesh);
	
	XprRenderTarget_releaseBuffer(_rt, color);
	XprRenderTarget_releaseBuffer(_rt, depth);

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
	PEZ_ENABLE_MULTISAMPLING = 1;
	PEZ_VERTICAL_SYNC = 0;
	PEZ_FORWARD_COMPATIBLE_GL = 1;
}

void PezExit(void)
{
	Cloth_free(_cloth);
	Mesh_free(_ballMesh);
	Mesh_free(_floorMesh);
	Mesh_free(_bgMesh);
	Material_free(_sceneMaterial);
	Material_free(_bgMaterial);
	Material_free(_uiMaterial);
	XprTexture_free(_texture);
	XprRenderTarget_free(_rt);
	Label_free(_label);
}

const char* PezInitialize(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	_aspect.width = (float)width;
	_aspect.height = (float)height;

	_mouse.isDown = XprFalse;

	_rt = XprRenderTarget_alloc();
	XprRenderTarget_init(_rt, (size_t)width, (size_t)height);

	_label = Label_alloc();
	Label_init(_label, width, height);

	// materials
	glswInit();
	glswSetPath("../example/", ".glsl");
	glswAddDirectiveToken("","#version 150");

	_sceneMaterial = loadMaterial(
		"ClothSimulation.Scene.Vertex",
		"ClothSimulation.Scene.Fragment");
	
	_bgMaterial = loadMaterial(
		"ClothSimulation.Bg.Vertex",
		"ClothSimulation.Bg.Fragment");
	
	_uiMaterial = loadMaterial(
		"ClothSimulation.Ui.Vertex",
		"ClothSimulation.Ui.Fragment");

	_texture = Pvr_createTexture(red_tile_texture);
	
	glswShutdown();

	{
	XprVec3 offset = XprVec3_(-1, 1.5f, 0);
	_cloth = Cloth_new(2, 2, &offset, 32);
	}

	_ball[0].center = XprVec3_(-0.5f, 0.5f, 0);
	_ball[0].radius = 0.25f;
	_ball[1].center = XprVec3_(0.5f, 0.5f, 0);
	_ball[1].radius = 0.25f;
	_ballMesh = Mesh_alloc();
	Mesh_initWithUnitSphere(_ballMesh, 32);

	{
	XprVec3 offset = XprVec3_(-2.5f, -2.5f, 0);
	_floorMesh = Mesh_alloc();
	Mesh_initWithQuad(_floorMesh, 5, 5, &offset, 1);
	}

	_bgMesh = Mesh_alloc();
	Mesh_initWithScreenQuad(_bgMesh);
	
	atexit(PezExit);

	return "Cloth Simulation";
}
/*
void mouse(int button, int state, int x, int y)
{
	_mouse.button = button;
	_mouse.state = state;
	_mouse.x = x;
	_mouse.y = y;

	_mouse.clothOffsets[0] = _cloth->fixPos[0];
	_mouse.clothOffsets[1] = _cloth->fixPos[_cloth->segmentCount-1];
}

void motion(int x, int y)
{
	int dx = x - _mouse.x;
	int dy = y - _mouse.y;

	if(_mouse.state == GLUT_DOWN && _mouse.button == GLUT_LEFT_BUTTON)
	{
		float mouseSensitivity = 0.0025f;
		_cloth->fixPos[0].x = _mouse.clothOffsets[0].x + dx * mouseSensitivity;
		_cloth->fixPos[_cloth->segmentCount-1].x = _mouse.clothOffsets[1].x + dx * mouseSensitivity;

		_cloth->fixPos[0].y = _mouse.clothOffsets[0].y + dy * -mouseSensitivity;
		_cloth->fixPos[_cloth->segmentCount-1].y = _mouse.clothOffsets[1].y + dy * -mouseSensitivity;
	}
}
*/
