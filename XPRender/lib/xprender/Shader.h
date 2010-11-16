#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// xprShader
typedef enum xprShaderType
{
	xprShaderType_Vertex,
	xprShaderType_Geometry,
	xprShaderType_Fragment,
} xprShaderType;

typedef enum xprShaderFlag
{
	xprShaderFlag_Compiled = 1 << 0,
} xprShaderFlag;

typedef struct xprShader
{
	int name;
	xprShaderType type;
	size_t flags;

} xprShader;

xprShader* xprShader_new(const char** sources, size_t srcCnt, xprShaderType type);

void xprShader_free(xprShader* self);

// xprShadingProgram
typedef enum xprShadingProgramFlag
{
	xprShadingProgramFlag_Linked = 1 << 0,
} xprShadingProgramFlag;

typedef struct xprShadingProgram
{
	int name;
	size_t flags;

} xprShadingProgram;

xprShadingProgram* xprShadingProgram_new(const xprShader** const shaders, size_t shaderCnt);

void xprShadingProgram_free(xprShadingProgram* self);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__