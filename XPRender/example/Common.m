#include "Common.h"
#include "../xprender/Memory.h"

#include "../lib/glsw/glsw.h"

#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSData.h>

typedef struct iosFILE
{
	size_t offset;
	NSData* data;
} iosFILE;

void* iosOpen(const char* filename)
{
	NSString* path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String: filename] ofType:nil];

	if(nil == path)
		return nil;

	NSData* data = [[NSData dataWithContentsOfFile:path] retain];

	if(nil == data)
		return nil;

	iosFILE* file = xprMemory()->alloc(sizeof(iosFILE), "io");
	file->offset = 0;
	file->data = data;
	return file;
}

void iosClose(void* handle)
{
	if(nil == handle)
		return;

	iosFILE* file = handle;
	[file->data release];
	xprMemory()->free(file, "io");
}

size_t iosRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	//return (size_t)AAsset_read((AAsset*)handle, buff, elsize * nelem);
	iosFILE* file = handle;

	NSRange range;
	range.location = file->offset;
	range.length = elsize * nelem;

	if(range.location + range.length >= [file->data length]) {
		range.length = ([file->data length]-1) - range.location;
	}

	if(range.length > 0) {
		[file->data getBytes: buff range:range];
		file->offset += range.length;
	}

	return range.length;
}

glswFileSystem myFileSystem = {iosRead, iosOpen, iosClose};
InputStream myInputStream = {iosRead, iosOpen, iosClose};

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

	self->renderTarget = xprRenderTargetAlloc();
	xprRenderTargetInit(self->renderTarget);

	self->inputStream = &myInputStream;

	self->aspect.width = (float)xprAppContext.xres;
	self->aspect.height = (float)xprAppContext.yres;

	xprRenderTargetSetViewport(0, 0, self->aspect.width, self->aspect.height, -1, 1);

	xprDbgStr("xprender started with %d x %d, api=%s\n", xprAppContext.xres, xprAppContext.yres, xprAppContext.apiName);
}

void appFree(AppContext* self)
{
	xprRenderTargetFree(self->renderTarget);
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
	else if(strcmp("gles", xprAppContext.apiName) == 0) {
		glswSetPath("", ".gles");
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


