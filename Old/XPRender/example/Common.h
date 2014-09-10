#ifndef __EXAMPLE_COMMON_H__
#define __EXAMPLE_COMMON_H__

#include "../lib/xprender/Platform.h"
#include "../lib/xprender/Framework.h"
#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"
#include "../lib/xprender/Mat44.h"
#include "../lib/xprender/Shader.h"
#include "../lib/xprender/GpuState.h"
#include "../lib/xprender/RenderTarget.h"

#include "Material.h"
#include "Stream.h"

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ShaderContext
{
	XprMat44 worldViewProjMtx;
	XprMat44 worldViewMtx;
	XprMat44 worldMtx;
	XprVec4 matDiffuse;
	XprVec4 matSpecular;
	float matShininess;
} ShaderContext;

typedef struct AppContext
{
	struct
	{
		float width;
		float height;
	} aspect;

	ShaderContext shaderContext;

	XprGpuState* gpuState;

	XprRenderTarget* renderTarget;

	InputStream* inputStream;

} AppContext;

AppContext* appAlloc();
void appInit(AppContext* sel);
void appFree(AppContext* self);

void appLoadMaterialBegin(AppContext* self, const char** directives);
Material* appLoadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey);
void appLoadMaterialEnd(AppContext* self);

void appShaderContextPreRender(AppContext* self, Material* material);

#ifdef __cplusplus
}
#endif


#endif	// __EXAMPLE_COMMON_H__
