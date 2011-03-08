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

AppContext* app = nullptr;
RemoteConfig* config = nullptr;

Cloth* cloth = nullptr;

#define BallCount 2
Sphere ball[BallCount];
Mesh* ballMesh = nullptr;

Mesh* floorMesh = nullptr;
Mesh* bgMesh = nullptr;

Material* sceneMtl = nullptr;
Material* bgMtl = nullptr;
XprTexture* texture = nullptr;

typedef struct Settings
{
	float gravity;
	float airResistance;
	float impact;
} Settings;

Settings settings = {10, 5, 3};

typedef struct Mouse
{
	XprBool isDown;
	int x;
	int y;
	XprVec3 clothOffsets[2];
	
} Mouse;

Mouse mouse = {0};

void drawBackground()
{
	static const XprVec4 c[] = {
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.145f, 0.31f, 0.405f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
	};
	XprGpuStateDesc* gpuState = &app->gpuState->desc;

	gpuState->depthTest = XprFalse;
	gpuState->cull = XprTrue;
	xprGpuStatePreRender(app->gpuState);

	xprGpuProgramPreRender(bgMtl->program);
	xprGpuProgramUniform4fv(bgMtl->program, XprHash("u_colors"), 4, (const float*)c);

	meshPreRender(bgMesh, bgMtl->program);
	meshRenderTriangles(bgMesh);
}

void drawScene()
{
	XprVec3 eyeAt = xprVec3(-2.5f, 1.5f, 5);
	XprVec3 lookAt = xprVec3(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;
	XprMat44 projMtx;
	XprMat44 viewProjMtx;

	XprGpuStateDesc* gpuState = &app->gpuState->desc;
	
	xprMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	xprMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	gpuState->cull = XprTrue;
	gpuState->depthTest = XprTrue;
	xprGpuStatePreRender(app->gpuState);

	xprGpuProgramPreRender(sceneMtl->program);
	xprGpuProgramUniformTexture(sceneMtl->program, XprHash("u_tex"), texture);

	// draw floor
	{	
		//app->shaderContext.matDiffuse = xprVec4(1.0f, 0.88f, 0.33f, 1);
		app->shaderContext.matDiffuse = xprVec4(1.0f, 1.0f, 1.0f, 1);
		app->shaderContext.matSpecular = xprVec4(0, 0, 0, 1);
		app->shaderContext.matShininess = 32;
		{
			XprMat44 m;
			XprVec3 axis = {1, 0, 0};
			xprMat44MakeRotation(&m, &axis, -90);
			
			xprMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		appShaderContextPreRender(app, sceneMtl);

		meshPreRender(floorMesh, sceneMtl->program);
		meshRenderTriangles(floorMesh);
	}

	// draw cloth
	{	
		app->shaderContext.matDiffuse = xprVec4(1.0f, 0.22f, 0.0f, 1);
		app->shaderContext.matSpecular = xprVec4(0.125f, 0.125f, 0.125f, 1);
		app->shaderContext.matShininess = 32;

		gpuState->cull = XprFalse;
		xprGpuStatePreRender(app->gpuState);
		{
			XprMat44 m;
			xprMat44SetIdentity(&m);

			xprMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		appShaderContextPreRender(app, sceneMtl);

		meshPreRender(cloth->mesh, sceneMtl->program);
		meshRenderTriangles(cloth->mesh);
		
		gpuState->cull = XprTrue;
		xprGpuStatePreRender(app->gpuState);
	}

	// draw balls
	{	
		int i;
		app->shaderContext.matDiffuse = xprVec4(0.9f, 0.64f, 0.35f, 1);
		app->shaderContext.matSpecular = xprVec4(1, 1, 1, 1);
		app->shaderContext.matShininess = 32;

		for(i=0; i<BallCount; ++i) {
			XprMat44 m;
			XprVec3 scale = {ball[i].radius, ball[i].radius, ball[i].radius};
			xprMat44MakeScale(&m, &scale);
			xprMat44SetTranslation(&m, &ball[i].center);
			
			xprMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);

			appShaderContextPreRender(app, sceneMtl);
			
			meshPreRender(ballMesh, sceneMtl->program);
			meshRenderTriangles(ballMesh);
		}
	}
}

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings lsettings;

	int iter;
	XprVec3 f;

	remoteConfigLock(config);
	lsettings = settings;
	remoteConfigUnlock(config);

	t += 0.0005f * lsettings.impact;

	ball[0].center.z = cosf(t) * 5.f;
	ball[1].center.z = sinf(t) * 5.f;

	cloth->timeStep = 0.01f;	// fixed time step
	cloth->damping = lsettings.airResistance * 1e-3f;

	// perform relaxation
	for(iter = 0; iter < 5; ++iter)
	{
		XprVec3 floorN = {0, 1, 0};
		XprVec3 floorP = {0, 0, 0};

		int i;
		for(i=0; i<BallCount; ++i)
			Cloth_collideWithSphere(cloth, &ball[i]);

		Cloth_collideWithPlane(cloth, &floorN, &floorP);
		Cloth_satisfyConstraints(cloth);
	}
	
	f = xprVec3(0, -lsettings.gravity * cloth->timeStep, 0);
	Cloth_addForceToAll(cloth, &f);

	Cloth_verletIntegration(cloth);

	Cloth_updateMesh(cloth);
}

void xprAppHandleMouse(int x, int y, int action)
{
	if(XprApp_MouseDown == action) {
		mouse.x = x;
		mouse.y = y;

		mouse.clothOffsets[0] = cloth->fixPos[0];
		mouse.clothOffsets[1] = cloth->fixPos[cloth->segmentCount-1];

		mouse.isDown = XprTrue;
	}
	else if(XprApp_MouseUp == action) {
		mouse.isDown = XprFalse;
	}
	else if((XprApp_MouseMove == action) && (XprTrue == mouse.isDown)) {
		int dx = x - mouse.x;
		int dy = y - mouse.y;
		
		float mouseSensitivity = 0.0025f;
		cloth->fixPos[0].x = mouse.clothOffsets[0].x + dx * mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].x = mouse.clothOffsets[1].x + dx * mouseSensitivity;

		cloth->fixPos[0].y = mouse.clothOffsets[0].y + dy * -mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].y = mouse.clothOffsets[1].y + dy * -mouseSensitivity;
	}
}

void xprAppRender()
{
	xprRenderTargetClearDepth(1);

	drawBackground();
	drawScene();
}

void xprAppConfig()
{
	xprAppContext.appName = "Cloth Simulation";
	xprAppContext.xres = 800;
	xprAppContext.yres = 600;
	xprAppContext.multiSampling = XprTrue;
	xprAppContext.vsync = XprFalse;
	xprAppContext.apiMajorVer = 3;
	xprAppContext.apiMinorVer = 3;
}

void xprAppFinalize()
{
	remoteConfigFree(config);
	Cloth_free(cloth);
	meshFree(ballMesh);
	meshFree(floorMesh);
	meshFree(bgMesh);
	materialFree(sceneMtl);
	materialFree(bgMtl);
	xprTextureFree(texture);
	appFree(app);
}

XprBool xprAppInitialize()
{
	app = appAlloc();
	appInit(app);
	
	// remote config
	{
		RemoteVarDesc descs[] = {
			{"gravity", &settings.gravity, 1, 100},
			{"airResistance", &settings.airResistance, 1, 20},
			{"impact", &settings.impact, 1, 10},
			{nullptr, nullptr, 0, 0}
		};
		
		config = remoteConfigAlloc();
		remoteConfigInit(config, 80, XprTrue);
		remoteConfigAddVars(config, descs);
	}

	// materials
	{
		const char* directives[]  = {nullptr};
		
		appLoadMaterialBegin(app, directives);

		sceneMtl = appLoadMaterial(
			"ClothSimulation.Scene.Vertex",
			"ClothSimulation.Scene.Fragment",
			nullptr, nullptr, nullptr);
		
		bgMtl = appLoadMaterial(
			"Common.Bg.Vertex",
			"Common.Bg.Fragment",
			nullptr, nullptr, nullptr);
		
		appLoadMaterialEnd(app);
	}

	// textures
	{
		texture = Pvr_createTexture(red_tile_texture);
	}

	// cloth
	{
		XprVec3 offset = xprVec3(-1, 1.5f, 0);
		cloth = Cloth_new(2, 2, &offset, 32);
	}

	// balls
	{
		ball[0].center = xprVec3(-0.5f, 0.5f, 0);
		ball[0].radius = 0.25f;
		ball[1].center = xprVec3(0.5f, 0.5f, 0);
		ball[1].radius = 0.25f;
		ballMesh = meshAlloc();
		meshInitWithUnitSphere(ballMesh, 32);
	}

	// floor
	{
		XprVec3 offset = xprVec3(-2.5f, -2.5f, 0);
		floorMesh = meshAlloc();
		meshInitWithQuad(floorMesh, 5, 5, &offset, 1);
	}

	// bg
	{
		bgMesh = meshAlloc();
		meshInitWithScreenQuad(bgMesh);
	}

	return XprTrue;
}
