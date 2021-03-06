#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Remote.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;

Mesh* tessMesh = nullptr;
Mesh* bgMesh = nullptr;
Material* tessMtl = nullptr;
Material* bgMtl = nullptr;

typedef struct Settings
{
	float tessLevel;
	float linearity;
} Settings;

Settings settings;

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

void drawScene(Settings* settings)
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
	xprMat44AdjustToAPIDepthRange(&projMtx);
	xprMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	gpuState->depthTest = XprTrue;
	gpuState->cull = XprTrue;
	//gpuState->polygonMode = XprGpuState_PolygonMode_Line;
	xprGpuStatePreRender(app->gpuState);

	xprGpuProgramPreRender(tessMtl->program);
	
	// draw floor
	app->shaderContext.matDiffuse = xprVec4(1.0f, 0.88f, 0.33f, 1);
	app->shaderContext.matSpecular = xprVec4(2, 2, 2, 1);
	app->shaderContext.matShininess = 32;

	{
		XprMat44 m;
		XprVec3 axis = {1, 0, 0};
		xprMat44MakeRotation(&m, &axis, -90);
		
		xprMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
		xprMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
	}

	appShaderContextPreRender(app, tessMtl);

	xprGpuProgramUniform1fv(tessMtl->program, XprHash("u_tessLevel"), 1, (const float*)&(settings->tessLevel));
	xprGpuProgramUniform1fv(tessMtl->program, XprHash("u_linearity"), 1, (const float*)&(settings->linearity));

	meshPreRender(tessMesh, tessMtl->program);
	meshRenderPatches(tessMesh);

	gpuState->polygonMode = XprGpuState_PolygonMode_Fill;
}

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
}

void xprAppHandleMouse(int x, int y, int action)
{
}

void xprAppRender()
{
	Settings localSettings;

	remoteConfigLock(config);
	localSettings = settings;
	localSettings.linearity /= 10.0f;
	remoteConfigUnlock(config);

	xprRenderTargetClearDepth(1);

	drawBackground();
	drawScene(&localSettings);
}

void xprAppConfig()
{
	xprAppContext.appName = "Triangle Tessellation";
	xprAppContext.xres = 800;
	xprAppContext.yres = 600;
	xprAppContext.multiSampling = XprTrue;
	xprAppContext.vsync = XprFalse;
	xprAppContext.apiMajorVer = 4;
	xprAppContext.apiMinorVer = 0;
}

void xprAppFinalize()
{
	meshFree(tessMesh);
	meshFree(bgMesh);
	materialFree(tessMtl);
	materialFree(bgMtl);
	remoteConfigFree(config);
	appFree(app);
}

XprBool xprAppInitialize()
{
	app = appAlloc();
	appInit(app);

	settings.tessLevel = 8;
	settings.linearity = 3;

	// remote config
	{
		RemoteVarDesc descs[] = {
			{"tessLevel", &settings.tessLevel, 0, 16},
			{"linearity", &settings.linearity, 0, 10},
			{nullptr, nullptr, 0, 0},
		};

		config = remoteConfigAlloc();
		remoteConfigInit(config, 8080, XprTrue);
		remoteConfigAddVars(config, descs);
	}

	// materials
	{
		const char* directives[]  = {nullptr};
		
		appLoadMaterialBegin(app, directives);

		tessMtl = appLoadMaterial(
			"TriangleTessellation.Scene.Vertex",
			"TriangleTessellation.Scene.Fragment",
			"TriangleTessellation.Scene.TessControl",
			"TriangleTessellation.Scene.TessEvaluation",
			nullptr);
		
		bgMtl = appLoadMaterial(
			"Common.Bg.Vertex",
			"Common.Bg.Fragment",
			nullptr, nullptr, nullptr);
		
		appLoadMaterialEnd(app);
	}

	// meshs
	{
		tessMesh = meshAlloc();
		meshInitWithObjFile(tessMesh, "monkey.obj", app->inputStream);

		bgMesh = meshAlloc();
		meshInitWithScreenQuad(bgMesh);
	}

	return XprTrue;
}
