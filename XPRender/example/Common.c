#include "Common.h"

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

	FILE* fp;

	if(nullptr != (fp = fopen(filename, "rb")))
		return fp;

	strcpy(buf, "../example/");
	if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
		return fp;

	strcpy(buf, "../media/");
	if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
		return fp;

	strcpy(buf, "media/");
	if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
		return fp;

	return nullptr;
}

void myClose(void* handle)
{
	fclose((FILE*)handle);
}

glswFileSystem myFileSystem = {fread, myOpen, myClose};
InputStream myInputStream = {fread, myOpen, myClose};

#endif	// XPR_ANDROID

void RenderContext_preRender(RenderContext* self, Material* material)
{
	xprGpuProgramUniformMtx4fv(material->program, XprHash("u_worldViewMtx"), 1, XprTrue, self->worldViewMtx.v);
	xprGpuProgramUniformMtx4fv(material->program, XprHash("u_worldViewProjMtx"), 1, XprTrue, self->worldViewProjMtx.v);
	xprGpuProgramUniformMtx4fv(material->program, XprHash("u_worldMtx"), 1, XprTrue, self->worldMtx.v);
	xprGpuProgramUniform4fv(material->program, XprHash("u_matDiffuse"), 1, self->matDiffuse.v);
	xprGpuProgramUniform4fv(material->program, XprHash("u_matSpecular"), 1, self->matSpecular.v);
	xprGpuProgramUniform1fv(material->program, XprHash("u_matShininess"), 1, &self->matShininess);
}


Material* loadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey)
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
	
	material = Material_alloc();
	
	Material_initWithShaders(material, args);

	if(0 == (material->flags & MaterialFlag_Inited))
		PezDebugString("failed to load material vs=%s,fs=%s!\n", vsKey, fsKey);

	return material;
}
