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

void PezUpdate(unsigned int elapsedMilliseconds)
{
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	static float t = 0.0f;
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
	
	t += 0.01f;
	if(t > 1.0f) {
		t = 0.0f;
	}
	
	xprMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
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

void PezConfig()
{
	PEZ_GL_VERSION_MAJOR = 2;
	PEZ_GL_VERSION_MINOR = 1;
}

void PezFinalize()
{
	meshFree(mesh);
	materialFree(mtl);
	//remoteConfigFree(config);
	appFree(app);
}

const char* PezInitialize(int width, int height)
{
	const char* appName = "Android Example";
	
	app = appAlloc();
	appInit(app, (float)width, (float)height);

	// remote config
	{
		RemoteVarDesc descs[] = {
			{"size", &settings.size, 1, 100},
			{nullptr, nullptr, 0, 0}
		};
		
		config = remoteConfigAlloc();
		//remoteConfigInit(config, 80, XprTrue);
		//remoteConfigAddVars(config, descs);
	}
	
	// load mesh
	{
		mesh = meshAlloc();
		if(!meshInitWithObjFile(mesh, "monkey.obj", app->inputStream))
			return appName;
	}
	
	// load materials
	{
		appLoadMaterialBegin(app, nullptr);

		mtl = appLoadMaterial(
			"Android.Scene.Vertex",
			"Android.Scene.Fragment",
			nullptr, nullptr, nullptr);
		if(0 == (mtl->flags & MaterialFlag_Inited))
			return appName;
		
		appLoadMaterialEnd(app);
	}
	
	bgClr = xprVec4(0.25f, 1, 0.25f, 1);
	
	XprDbgStr("XpRender Android example started");

	return appName;
}
