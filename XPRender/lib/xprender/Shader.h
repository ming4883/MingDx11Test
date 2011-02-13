#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"
#include "StrHash.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprTexture;
typedef struct XprTexture XprTexture;

// XprGpuShader
typedef enum XprGpuShaderType
{
	XprGpuShaderType_Vertex,
	XprGpuShaderType_Fragment,
	XprGpuShaderType_Geometry,
	XprGpuShaderType_TessControl,
	XprGpuShaderType_TessEvaluation,
} XprGpuShaderType;

typedef enum XprGpuShaderFlag
{
	XprGpuShaderFlag_Compiled = 1 << 0,
} XprGpuShaderFlag;

struct XprGpuShaderImpl;

typedef struct XprGpuShader
{
	size_t flags;
	XprGpuShaderType type;
	struct XprGpuShaderImpl* impl;

} XprGpuShader;


XprGpuShader* xprGpuShaderAlloc();

void xprGpuShaderFree(XprGpuShader* self);

void xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type);

// XprGpuProgram
typedef enum XprGpuProgramFlag
{
	XprGpuProgramFlag_Linked = 1 << 0,
} XprGpuProgramFlag;

struct XprGpuProgramImpl;

typedef struct XprGpuProgram
{
	size_t flags;
	struct XprGpuProgramImpl* impl;

} XprGpuProgram;

XprGpuProgram* xprGpuProgramAlloc();

void xprGpuProgramFree(XprGpuProgram* self);

void xprGpuProgramInit(XprGpuProgram* self, const XprGpuShader** const shaders, size_t shaderCnt);

void xprGpuProgramPreRender(XprGpuProgram* self);

void xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
void xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
void xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
void xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
void xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value);
void xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, XprTexture* texture);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__