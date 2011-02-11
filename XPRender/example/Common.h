#ifndef __EXAMPLE_COMMON_H__
#define __EXAMPLE_COMMON_H__

#include "../lib/xprender/Platform.h"

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"
#include "../lib/xprender/Mat44.h"
#include "../lib/xprender/Shader.h"
#include "../lib/xprender/GpuState.h"
#include "../lib/xprender/RenderTarget.h"
#include "../lib/glsw/glsw.h"
#include "../lib/pez/pez.h"

#include "Material.h"
#include "Stream.h"

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RenderContext
{
	XprMat44 worldViewProjMtx;
	XprMat44 worldViewMtx;
	XprMat44 worldMtx;
	XprVec4 matDiffuse;
	XprVec4 matSpecular;
	float matShininess;

} RenderContext;

void RenderContext_preRender(RenderContext* self, Material* material);

extern glswFileSystem myFileSystem;
extern InputStream myInputStream;

Material* loadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey);

#ifdef __cplusplus
}
#endif


#endif	// __EXAMPLE_COMMON_H__
