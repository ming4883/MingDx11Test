#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"
#include "StrHash.h"
#include "GpuFormat.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprTexture;
struct XprBuffer;

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
	XprGpuShader_Inited = 1 << 0,
} XprGpuShaderFlag;

typedef struct XprGpuShader
{
	size_t flags;
	XprGpuShaderType type;

} XprGpuShader;


XprGpuShader* xprGpuShaderAlloc();

void xprGpuShaderFree(XprGpuShader* self);

XprBool xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type);

// XprGpuProgram
typedef enum XprGpuProgramFlag
{
	XprGpuProgram_Inited = 1 << 0,
} XprGpuProgramFlag;

typedef struct XprGpuProgram
{
	size_t flags;

} XprGpuProgram;

XprGpuProgram* xprGpuProgramAlloc();

void xprGpuProgramFree(XprGpuProgram* self);

XprBool xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt);

void xprGpuProgramPreRender(XprGpuProgram* self);

XprBool xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value);
XprBool xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value);
XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture);

typedef struct XprGpuProgramInput
{
	struct XprBuffer* buffer;
	const char* name;
	size_t offset;
	XprGpuFormat format;
} XprGpuProgramInput;

void xprGpuProgramBindInput(XprGpuProgram* self, XprGpuProgramInput* inputs, size_t count);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__