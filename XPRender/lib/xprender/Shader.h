#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// XprGpuShader
typedef enum XprGpuShaderType
{
	XprGpuShaderType_Vertex,
	XprGpuShaderType_Geometry,
	XprGpuShaderType_Fragment,
} XprGpuShaderType;

typedef enum XprGpuShaderFlag
{
	XprGpuShaderFlag_Compiled = 1 << 0,
} XprGpuShaderFlag;

typedef struct XprGpuShaderImpl;

typedef struct XprGpuShader
{
	size_t flags;
	XprGpuShaderType type;
	struct XprGpuShaderImpl* impl;

} XprGpuShader;


XprGpuShader* XprGpuShader_alloc();

void XprGpuShader_free(XprGpuShader* self);

void XprGpuShader_init(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type);

// XprGpuProgram
typedef enum XprGpuProgramFlag
{
	XprGpuProgramFlag_Linked = 1 << 0,
} XprGpuProgramFlag;

typedef struct XprGpuProgramImpl;

typedef struct XprGpuProgram
{
	size_t flags;
	struct XprGpuProgramImpl* impl;

} XprGpuProgram;

XprGpuProgram* XprGpuProgram_alloc();

void XprGpuProgram_free(XprGpuProgram* self);

void XprGpuProgram_init(XprGpuProgram* self, const XprGpuShader** const shaders, size_t shaderCnt);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__