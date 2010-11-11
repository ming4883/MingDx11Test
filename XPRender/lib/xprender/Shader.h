#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xprShaderType
{
	xprShaderType_Vertex,
	xprShaderType_Geometry,
	xprShaderType_Fragment,
} xprShaderType;

typedef struct xprShader
{
	int name;
	xprShaderType type;
	size_t flags;	// combinations of xprBufferFlag

} xprShader;

xprShader* xprShader_new(const char** sources, size_t srcCnt, xprShaderType type);

void xprShader_free(xprShader* self);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__