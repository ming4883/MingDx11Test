#include "Common.h"
#include "Mesh.h"

XprVec4 bgClr = {1, 0.25f, 0.25f, 1};
Mesh* mesh = nullptr;
Material* mtlScene = nullptr;

XprGpuState* gpuState = nullptr;

typedef struct Aspect
{
	float width;
	float height;
} Aspect;

Aspect aspect;

RenderContext renderContext;

void PezUpdate(unsigned int elapsedMilliseconds)
{
}

void PezHandleMouse(int x, int y, int action)
{
}

void PezRender()
{
	XprVec3 eyeAt = xprVec3(-2.5f, 1.5f, 5);
	XprVec3 lookAt = xprVec3(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;
	XprMat44 projMtx;
	XprMat44 viewProjMtx;
	
	xprMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44Prespective(&projMtx, 45.0f, aspect.width / aspect.height, 0.1f, 30.0f);
	xprMat44Mult(&viewProjMtx, &projMtx, &viewMtx);
		
	// clear
	{
		xprRenderTargetClearColor(bgClr.x, bgClr.y, bgClr.z, bgClr.w);
		xprRenderTargetClearDepth(1);
	}
	
	// draw scene
	{
		xprGpuStateSetDepthTestEnabled(gpuState, XprTrue);
		xprGpuStateSetCullEnabled(gpuState, XprTrue);
		xprGpuStatePreRender(gpuState);

		xprGpuProgramPreRender(mtlScene->program);
		
		renderContext.matDiffuse = xprVec4(1.0f, 0.88f, 0.33f, 1);
		renderContext.matSpecular = xprVec4(2, 2, 2, 1);
		renderContext.matShininess = 32;

		{
			XprMat44 m;
			XprVec3 axis = {1, 0, 0};
			xprMat44MakeRotation(&m, &axis, -90);
			
			xprMat44Mult(&renderContext.worldViewMtx, &viewMtx, &m);
			xprMat44Mult(&renderContext.worldViewProjMtx, &viewProjMtx, &m);
		}

		RenderContext_preRender(&renderContext, mtlScene);

		Mesh_preRender(mesh, mtlScene->program);
		Mesh_render(mesh);
	}
	/**/
}

void PezConfig()
{
}

void PezExit(void)
{
	Mesh_free(mesh);
	Material_free(mtlScene);
	xprGpuStateFree(gpuState);
}

const char* PezInitialize(int width, int height)
{
	const char* appName = "Android Example";
	
	aspect.width = (float)width;
	aspect.height = (float)height;
	
	gpuState = xprGpuStateAlloc();
	xprGpuStateInit(gpuState);
	
	// load mesh
	{
		mesh = Mesh_alloc();
		if(!Mesh_initWithObjFile(mesh, "monkey.obj", &myInputStream))
			return appName;
	}
	
	// load materials
	{
		glswInit(&myFileSystem);
		glswSetPath("", ".glsl");
		//glswAddDirectiveToken("","#version 400");
		
		mtlScene = loadMaterial(
			"Android.Scene.Vertex",
			"Android.Scene.Fragment",
			nullptr, nullptr, nullptr);
		if(0 == (mtlScene->flags & MaterialFlag_Inited))
			return appName;
		
		glswShutdown();
	}
	/**/
	
	bgClr = xprVec4(0.25f, 1, 0.25f, 1);
	
	XprDbgStr("XpRender Android example started");

	return appName;
}
