#ifndef __EXAMPLE_COMMON_H__
#define __EXAMPLE_COMMON_H__

#include "../lib/xprender/Platform.h"

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"
#include "../lib/xprender/Mat44.h"
#include "../lib/xprender/Shader.h"
#include "../lib/glsw/glsw.h"
#include "../lib/pez/pez.h"

#include "Material.h"

#include <GL/glew.h>

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RenderContext
{
	XprMat44 worldViewProjMtx;
	XprMat44 worldViewMtx;
	XprVec4 matDiffuse;
	XprVec4 matSpecular;
	float matShininess;

} RenderContext;

void RenderContext_apply(RenderContext* self, Material* material);

Material* loadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey);

#ifdef __cplusplus
}
#endif


#endif	// __EXAMPLE_COMMON_H__
