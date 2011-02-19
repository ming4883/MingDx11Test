#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"
#include "StrHash.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprTexture;

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

void xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt);

void xprGpuProgramPreRender(XprGpuProgram* self);

XprBool xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value);
XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__