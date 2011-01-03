#include <GL/glew.h>

#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"
#include "../lib/xprender/Mat44.h"
#include "../lib/xprender/Shader.h"
#include "../lib/glsw/glsw.h"
#include "../lib/pez/pez.h"

#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Material.h"

Cloth* _cloth = nullptr;
#define BallCount 2
Sphere _ball[BallCount];
Mesh* _ballMesh = nullptr;
Mesh* _floorMesh = nullptr;
Material* _sceneMaterial = nullptr;
Material* _uiMaterial = nullptr;
XprVec3 _floorN = {0, 1, 0};
XprVec3 _floorP = {0, 0, 0};
float _gravity = 50;
float _airResistance = 5;
float _impact = 3;
XprBool _showDebug = XprFalse;

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
	int locWorldView = glGetUniformLocation(material->pipeline->name, "u_worldViewMtx");
	int locWorldViewProj = glGetUniformLocation(material->pipeline->name, "u_worldViewProjMtx");
	int locMatDiffuse = glGetUniformLocation(material->pipeline->name, "u_matDiffuse");
	int locMatSpecular = glGetUniformLocation(material->pipeline->name, "u_matSpecular");
	int locMatShininess = glGetUniformLocation(material->pipeline->name, "u_matShininess");

	glUniformMatrix4fv(locWorldView, 1, XprTrue, self->worldViewMtx.v);
	glUniformMatrix4fv(locWorldViewProj, 1, XprTrue, self->worldViewProjMtx.v);
	glUniform4fv(locMatDiffuse, 1, self->matDiffuse.v);
	glUniform4fv(locMatSpecular, 1, self->matSpecular.v);
	glUniform1fv(locMatShininess, 1, &self->matShininess);
}

RenderContext _renderContext;

void drawBackground()
{
	/*
	static const XprVec3 v[] = {
		{-1,  1, 0},
		{-1, -1, 0},
		{ 1,  1, 0},
		{ 1, -1, 0},
	};

	static const XprVec3 c[] = {
		{0.57f, 0.85f, 1.0f},
		{0.29f, 0.62f, 0.81f},
		{0.57f, 0.85f, 1.0f},
		{0.57f, 0.85f, 1.0f},
	};

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_TRIANGLES);

	glColor3fv(c[0].v); glVertex3fv(v[0].v);
	glColor3fv(c[1].v);	glVertex3fv(v[1].v);
	glColor3fv(c[2].v); glVertex3fv(v[2].v);

	glColor3fv(c[3].v); glVertex3fv(v[3].v);
	glColor3fv(c[2].v); glVertex3fv(v[2].v);
	glColor3fv(c[1].v); glVertex3fv(v[1].v);

	glEnd();
	*/

	glClearColor(0.57f, 0.85f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
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

	glUseProgram(_sceneMaterial->pipeline->name);

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

		Mesh_bindInputs(_floorMesh, _sceneMaterial->pipeline);
		Mesh_draw(_floorMesh);
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

		Mesh_bindInputs(_cloth->mesh, _sceneMaterial->pipeline);
		Mesh_draw(_cloth->mesh);
		
		glEnable(GL_CULL_FACE);
	}

	{	// draw ball
		int i;
		_renderContext.matDiffuse = XprVec4_(0.9f, 0.64f, 0.35f, 1);
		_renderContext.matSpecular = XprVec4_(1, 1, 1, 1);
		_renderContext.matShininess = 32;

		for(i=0; i<BallCount; ++i)
		{
			XprMat44 m;
			XprVec3 scale = {_ball[i].radius, _ball[i].radius, _ball[i].radius};
			XprMat44_makeScale(&m, &scale);
			XprMat44_setTranslation(&m, &_ball[i].center);
			
			XprMat44_mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			XprMat44_mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);

			RenderContext_apply(&_renderContext, _sceneMaterial);

			Mesh_bindInputs(_ballMesh, _sceneMaterial->pipeline);
			Mesh_draw(_ballMesh);
		}
	}

	glUseProgram(0);

	if(XprTrue == _showDebug)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		
		glColor3f(1, 1, 1);

		{
			XprMat44 m;
			XprMat44_setIdentity(&m);
			XprMat44_mult(&_renderContext.worldViewMtx, &viewMtx, &m);
			XprMat44_mult(&_renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		RenderContext_apply(&_renderContext, _sceneMaterial);

		Mesh_drawPoints(_cloth->mesh);
	}
}

void quit(void)
{
	Cloth_free(_cloth);
	Mesh_free(_ballMesh);
	Mesh_free(_floorMesh);
	Material_free(_sceneMaterial);
	Material_free(_uiMaterial);
}

Material* loadMaterial(const char* vsKey, const char* fsKey)
{
	const char* args[] = {
		"vs", glswGetShader(vsKey),
		"fs", glswGetShader(fsKey),
		nullptr,
	};
	
	Material* material = Material_new(args);

	if(0 == (material->flags & MaterialFlag_Ready))
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
	_mouse.x = x;
	_mouse.y = y;

	_mouse.clothOffsets[0] = _cloth->fixPos[0];
	_mouse.clothOffsets[1] = _cloth->fixPos[_cloth->segmentCount-1];
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
	PEZ_ENABLE_MULTISAMPLING = 1;
	PEZ_VERTICAL_SYNC = 0;
	PEZ_FORWARD_COMPATIBLE_GL = 1;
}

const char* PezInitialize(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	_aspect.width = (float)width;
	_aspect.height = (float)height;

	// materials
	glswInit();
	glswSetPath("../example/", ".glsl");
	glswAddDirectiveToken("","#version 150");

	_sceneMaterial = loadMaterial(
		"ClothSimulation.Scene.Vertex",
		"ClothSimulation.Scene.Fragment");
	
	//_uiMaterial = loadMaterial(
	//	"ClothSimulation.UI.Vertex",
	//	"ClothSimulation.UI.Fragment");
	
	glswShutdown();

	{
	XprVec3 offset = XprVec3_(-1, 1.5f, 0);
	_cloth = Cloth_new(2, 2, &offset, 32);
	}

	_ball[0].center = XprVec3_(-0.5f, 0.5f, 0);
	_ball[0].radius = 0.25f;
	_ball[1].center = XprVec3_(0.5f, 0.5f, 0);
	_ball[1].radius = 0.25f;
	_ballMesh = Mesh_createUnitSphere(32);

	{
	XprVec3 offset = XprVec3_(-2.5f, -2.5f, 0);
	_floorMesh = Mesh_createQuad(5, 5, &offset, 1);
	}
	
	atexit(quit);

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
