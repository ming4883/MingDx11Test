#include "Common.h"
#include "Remote.h"
#include "Mesh.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;
XprVec4 bgClr = {1, 0.25f, 0.25f, 1};
Mesh* mesh = nullptr;
Material* mtl = nullptr;

typedef struct Settings
{
	float size;
} Settings;

Settings settings = {100};

float t = 0;

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
	t += elapsedMilliseconds * 0.0001f;
	if(t > 1.0f) {
		t = 0.0f;
	}
}

void xprAppHandleMouse(int x, int y, int action)
{
	xprDbgStr("handle mouse x=%d, y=%d, a=%d\n", x, y, action); 
}

void xprAppRender()
{
	XprVec3 eyeAt = xprVec3(-2.5f, 1.5f, 5);
	XprVec3 lookAt = xprVec3(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;
	XprMat44 projMtx;
	XprMat44 viewProjMtx;

	XprGpuStateDesc* gpuState = &app->gpuState->desc;

	Settings lsettings;

	remoteConfigLock(config);
	lsettings = settings;
	remoteConfigUnlock(config);
	
	xprMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	xprMat44AdjustToAPIDepthRange(&projMtx);
	xprMat44Mult(&viewProjMtx, &projMtx, &viewMtx);
		
	// clear
	{
		xprRenderTargetClearColor(bgClr.x, bgClr.y, bgClr.z, bgClr.w);
		xprRenderTargetClearDepth(1);
	}
	
	// draw scene
	{
		app->shaderContext.matDiffuse = xprVec4(1.0f, 0.88f, 0.33f, 1);
		app->shaderContext.matSpecular = xprVec4(2, 2, 2, 1);
		app->shaderContext.matShininess = 32;

		{
			XprVec3 axis = {1, 0, 0};
			XprVec3 scale = {lsettings.size / 100.f, lsettings.size / 100.f, lsettings.size / 100.f};
			XprMat44 r, s;
			xprMat44MakeScale(&s, &scale);
			xprMat44MakeRotation(&r, &axis, 360 * t);

			xprMat44Mult(&app->shaderContext.worldMtx, &r, &s);
			xprMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &app->shaderContext.worldMtx);
			xprMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &app->shaderContext.worldMtx);
		}

		gpuState->depthTest = XprTrue;
		gpuState->cull = XprTrue;
		xprGpuStatePreRender(app->gpuState);

		xprGpuProgramPreRender(mtl->program);
		appShaderContextPreRender(app, mtl);

		meshPreRender(mesh, mtl->program);
		meshRenderTriangles(mesh);
	}
}

void xprAppConfig()
{
	xprAppContext.appName = "Mesh";
	xprAppContext.xres = 480;
	xprAppContext.yres = 800;
	xprAppContext.multiSampling = XprFalse;
	xprAppContext.vsync = XprFalse;

	if(strcmp(xprAppContext.apiName, "gles") == 0) {
		xprAppContext.apiMajorVer = 2;
		xprAppContext.apiMinorVer = 0;
	}
	else {
		xprAppContext.apiMajorVer = 3;
		xprAppContext.apiMinorVer = 3;
	}
}

void xprAppFinalize()
{
	meshFree(mesh);
	materialFree(mtl);
	remoteConfigFree(config);
	appFree(app);
}

XprBool xprAppInitialize()
{
	app = appAlloc();
	appInit(app);

	// remote config
	{
		RemoteVarDesc descs[] = {
			{"size", &settings.size, 1, 100},
			{nullptr, nullptr, 0, 0}
		};
		
		config = remoteConfigAlloc();
		remoteConfigInit(config, 8080, XprTrue);
		remoteConfigAddVars(config, descs);
	}
	
	// load mesh
	{
		mesh = meshAlloc();
		if(!meshInitWithObjFile(mesh, "monkey.obj", app->inputStream))
			return XprFalse;
	}
	
	// load materials
	{
		appLoadMaterialBegin(app, nullptr);

		mtl = appLoadMaterial(
			"Mesh.Scene.Vertex",
			"Mesh.Scene.Fragment",
			nullptr, nullptr, nullptr);
		if(0 == (mtl->flags & MaterialFlag_Inited))
			return XprFalse;
		
		appLoadMaterialEnd(app);
	}
	
	bgClr = xprVec4(0.25f, 1, 0.25f, 1);
	
	xprDbgStr("XpRender Mesh example started");

	return XprTrue;
}
