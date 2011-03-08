#include "Common.h"

#include "../lib/glsw/glsw.h"

#if defined(XPR_ANDROID)

#include <android_native_app_glue.h>
#include <android/asset_manager.h>

extern struct android_app* PEZ_ANDROID_APP;

void* androidOpen(const char* filename)
{
	AAsset* asset = AAssetManager_open(PEZ_ANDROID_APP->activity->assetManager, filename, AASSET_MODE_UNKNOWN);
	return asset;
}

void androidClose(void* handle)
{
	AAsset_close((AAsset*)handle);
}

size_t androidRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	return (size_t)AAsset_read((AAsset*)handle, buff, elsize * nelem);
}

glswFileSystem myFileSystem = {androidRead, androidOpen, androidClose};
InputStream myInputStream = {androidRead, androidOpen, androidClose};

#else	// XPR_ANDROID

void* myOpen(const char* filename)
{
	static char buf[512];

	static const char* paths[] = {
		"../../example/",
		"../example/",
		"../../media/",
		"../media/",
		"media",
		nullptr,
	};

	
	FILE* fp;
	const char** path;

	if(nullptr != (fp = fopen(filename, "rb")))
		return fp;

	for(path = paths; nullptr != *path; ++path) {
		strcpy(buf, *path);
		if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
			return fp;
	}

	/*
	strcpy(buf, "../example/");
	if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
		return fp;

	strcpy(buf, "../media/");
	if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
		return fp;

	strcpy(buf, "media/");
	if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
		return fp;
	*/

	return nullptr;
}

void myClose(void* handle)
{
	fclose((FILE*)handle);
}

glswFileSystem myFileSystem = {fread, myOpen, myClose};
InputStream myInputStream = {fread, myOpen, myClose};

#endif	// XPR_ANDROID

AppContext* appAlloc()
{
	AppContext* self = malloc(sizeof(AppContext));
	memset(self, 0, sizeof(AppContext));
	return self;
}

void appInit(AppContext* self)
{
	self->gpuState = xprGpuStateAlloc();
	xprGpuStateInit(self->gpuState);

	self->inputStream = &myInputStream;

	self->aspect.width = (float)xprAppContext.xres;
	self->aspect.height = (float)xprAppContext.yres;

	xprRenderTargetSetViewport(0, 0, self->aspect.width, self->aspect.height, -1, 1);
	
	xprDbgStr("xprender started with %d x %d", xprAppContext.xres, xprAppContext.yres);
}

void appFree(AppContext* self)
{
	xprGpuStateFree(self->gpuState);
	free(self);
}

void appLoadMaterialBegin(AppContext* self, const char** directives)
{
	glswInit(&myFileSystem);

	if(strcmp("gl", xprAppContext.apiName) == 0) {
		glswSetPath("", ".glsl");
		if(3 == xprAppContext.apiMajorVer) {
			glswAddDirectiveToken("", "#version 150"); 
		}
		else if(4 == xprAppContext.apiMajorVer) {
			glswAddDirectiveToken("", "#version 400");
		}
	}
	else if(strcmp("d3d9", xprAppContext.apiName) == 0) {
		glswSetPath("", ".hlsl");
	}

	if(nullptr != directives) {
		int i = 0;
		const char* key;
		const char* val;

		while(1) {
			key = directives[i++];
			if(nullptr == key) break;

			val = directives[i++];
			if(nullptr == val) break;

			glswAddDirectiveToken(key, val);
		}
	}
}

void appLoadMaterialEnd(AppContext* self)
{
	glswShutdown();
}

Material* appLoadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey)
{
	const char* args[11] = {nullptr};
	size_t idx = 0;
	Material* material = nullptr;

	if(nullptr != vsKey) {
		args[idx++] = "vs";
		args[idx++] = glswGetShader(vsKey);
	}

	if(nullptr != tcKey) {
		args[idx++] = "tc";
		args[idx++] = glswGetShader(tcKey);
	}

	if(nullptr != teKey) {
		args[idx++] = "te";
		args[idx++] = glswGetShader(teKey);
	}

	if(nullptr != gsKey) {
		args[idx++] = "gs";
		args[idx++] = glswGetShader(gsKey);
	}
	
	if(nullptr != fsKey) {
		args[idx++] = "fs";
		args[idx++] = glswGetShader(fsKey);
	}
	
	material = materialAlloc();
	
	materialInitWithShaders(material, args);

	if(0 == (material->flags & MaterialFlag_Inited))
		xprDbgStr("failed to load material vs=%s,fs=%s!\n", vsKey, fsKey);

	return material;
}

void appShaderContextPreRender(AppContext* self, Material* material)
{
	ShaderContext shdcontext = self->shaderContext;
	
	// gles does not support transpose matrix, so we have to DIY
	xprMat44Transpose(&shdcontext.worldViewMtx, &shdcontext.worldViewMtx);
	xprMat44Transpose(&shdcontext.worldViewProjMtx, &shdcontext.worldViewProjMtx);
	xprMat44Transpose(&shdcontext.worldMtx, &shdcontext.worldMtx);
	
	xprGpuProgramUniformMtx4fv(material->program, XprHash("u_worldViewMtx"), 1, XprFalse, shdcontext.worldViewMtx.v);
	xprGpuProgramUniformMtx4fv(material->program, XprHash("u_worldViewProjMtx"), 1, XprFalse, shdcontext.worldViewProjMtx.v);
	xprGpuProgramUniformMtx4fv(material->program, XprHash("u_worldMtx"), 1, XprFalse, shdcontext.worldMtx.v);
	xprGpuProgramUniform4fv(material->program, XprHash("u_matDiffuse"), 1, shdcontext.matDiffuse.v);
	xprGpuProgramUniform4fv(material->program, XprHash("u_matSpecular"), 1, shdcontext.matSpecular.v);
	xprGpuProgramUniform1fv(material->program, XprHash("u_matShininess"), 1, &shdcontext.matShininess);
}


