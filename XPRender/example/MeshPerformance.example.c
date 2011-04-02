#include "Common.h"
#include "Remote.h"
#include "Mesh.h"
#include "Pvr.h"
#include "red_tile_texture.h"

AppContext* app = nullptr;
//RemoteConfig* config = nullptr;
XprVec4 bgClr = {0.25f, 0.25f, 0.25f, 1};
Mesh* mesh = nullptr;
Mesh* batchMesh = nullptr;
Material* mtl = nullptr;
XprTexture* texture = nullptr;

#define CNT 5
#define BATCH_DRAW 0

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
}

void xprAppRender()
{
	size_t row, col;
	XprVec3 eyeAt = xprVec3(0, 1.5f, 20);
	XprVec3 lookAt = xprVec3(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;
	XprMat44 projMtx;
	XprMat44 viewProjMtx;

	XprGpuStateDesc* gpuState = &app->gpuState->desc;

	Settings lsettings;

	//remoteConfigLock(config);
	lsettings = settings;
	//remoteConfigUnlock(config);
	
	xprMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	xprMat44AdjustToAPIDepthRange(&projMtx);
	xprMat44Mult(&viewProjMtx, &projMtx, &viewMtx);
		
	// clear
	{
		xprRenderTargetClearColor(bgClr.x, bgClr.y, bgClr.z, bgClr.w);
		xprRenderTargetClearDepth(1);
	}

	// setup gpu state
	{
		gpuState->depthTest = XprTrue;
		gpuState->cull = XprTrue;
		xprGpuStatePreRender(app->gpuState);

		xprGpuProgramPreRender(mtl->program);
		{	
			XprSampler sampler = {
				XprSamplerFilter_MagMinMip_Linear, 
				XprSamplerAddress_Wrap, 
				XprSamplerAddress_Wrap
			};
			xprGpuProgramUniformTexture(mtl->program, XprHash("u_tex"), texture, &sampler);
		}
	}

	// draw scene
	{
		XprVec3 scale = {lsettings.size / 100.f, lsettings.size / 100.f, lsettings.size / 100.f};
	
		XprMat44 m, r, s;
		xprMat44MakeScale(&s, &scale);
		xprMat44MakeRotation(&r, XprVec3_c100(), 360 * t);
		xprMat44Mult(&m, &r, &s);

		if(BATCH_DRAW) {
			
			app->shaderContext.worldViewProjMtx = viewProjMtx;
			appShaderContextPreRender(app, mtl);

			
			meshPreRender(batchMesh, mtl->program);
			meshRenderTriangles(batchMesh);
		}
		else {

			meshPreRender(mesh, mtl->program);

			for(row = 0; row < CNT; ++row) {
				for(col = 0; col < CNT; ++col) {

					XprVec3 pos = {col*2.0f-CNT, row*2.0f-CNT, 0};
					xprMat44SetTranslation(&m, &pos);

					xprMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
					appShaderContextPreRender(app, mtl);

					meshRenderTriangles(mesh);
				}
			}
		}
	}
}

void xprAppConfig()
{
	xprAppContext.appName = "MeshPerformance";
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
	meshFree(batchMesh);
	meshFree(mesh);
	xprTextureFree(texture);
	materialFree(mtl);
	//remoteConfigFree(config);
	appFree(app);
}

XprBool xprAppInitialize()
{
	app = appAlloc();
	appInit(app);

	/*
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
	*/
	
	// load mesh
	{
		mesh = meshAlloc();
		if(!meshInitWithObjFile(mesh, "monkey.obj", app->inputStream))
			return XprFalse;

		xprDbgStr("mesh vcnt=%d, icnt=%d\n", mesh->vertexCount, mesh->indexCount);
	}

	{
		size_t instCnt = CNT * CNT;
		batchMesh = meshAlloc();
		meshInit(batchMesh, mesh->vertexCount * instCnt, mesh->indexCount * instCnt);
	}

	{
		size_t row, col;
		size_t i;
		
		unsigned char* dstVert = batchMesh->vertex.buffer;
		unsigned char* srcVert = mesh->vertex.buffer;

		unsigned char* dstUV = batchMesh->texcoord[0].buffer;
		unsigned char* srcUV = mesh->texcoord[0].buffer;

		unsigned char* dstIdx = batchMesh->index.buffer;
		unsigned char* srcIdx = mesh->index.buffer;

		unsigned short idxOffset = 0;

		XprMat44 m;
		xprMat44SetIdentity(&m);
		
		for(row = 0; row < CNT; ++row) {
			for(col = 0; col < CNT; ++col) {

				XprVec3 pos = {col*2.0f-CNT, row*2.0f-CNT, 0};
				xprMat44SetTranslation(&m, &pos);

				xprMat44TransformAffinePts((XprVec3*)dstVert, (XprVec3*)srcVert, mesh->vertexCount, &m);
				dstVert += mesh->vertex.sizeInBytes;

				memcpy(dstUV, srcUV, mesh->texcoord[0].sizeInBytes);
				dstUV += mesh->texcoord[0].sizeInBytes;

				memcpy(dstIdx, srcIdx, mesh->index.sizeInBytes);
				
				{
					unsigned short* dst = (unsigned short*)dstIdx;
					unsigned short* src = (unsigned short*)srcIdx;
					for(i = 0; i<mesh->indexCount; ++i) {
						*dst = *src + idxOffset;
						++dst;
						++src;
					}
				}
				idxOffset += mesh->vertexCount;
				dstIdx += mesh->index.sizeInBytes;
			}
		}

		meshCommit(batchMesh);
	}
	
	// load materials
	{
		appLoadMaterialBegin(app, nullptr);

		mtl = appLoadMaterial(
			"MeshPerformance.Scene.Vertex",
			"MeshPerformance.Scene.Fragment",
			nullptr, nullptr, nullptr);
		if(0 == (mtl->flags & MaterialFlag_Inited))
			return XprFalse;
		
		appLoadMaterialEnd(app);
	}

	texture = Pvr_createTexture(red_tile_texture);
	
	xprDbgStr("XpRender MeshPerformance example started, batch=%d\n", BATCH_DRAW);

	return XprTrue;
}
