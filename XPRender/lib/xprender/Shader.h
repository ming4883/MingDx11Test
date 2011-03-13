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

typedef enum XprSamplerFilter
{
	XprSamplerFilter_MagMinMip_Nearest,
	XprSamplerFilter_MagMinMip_Linear,
	XprSamplerFilter_MagMin_Nearest_Mip_Linear,
	XprSamplerFilter_MagMin_Linear_Mip_Nearest,
	XprSamplerFilter_MagMin_Nearest_Mip_None,
	XprSamplerFilter_MagMin_Linear_Mip_None,

} XprSamplerFilter;

typedef enum XprSamplerAddress
{
	XprSamplerAddress_Wrap,
	XprSamplerAddress_Clamp,

} XprSamplerAddress;

typedef struct XprSampler
{
	XprSamplerFilter filter;
	XprSamplerAddress addressU, addressV, addressW;

} XprSampler;


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
XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture, const struct XprSampler* sampler);

typedef struct XprGpuProgramInput
{
	struct XprBuffer* buffer;
	const char* name;
	size_t offset;
	XprGpuFormat format;
} XprGpuProgramInput;

size_t xprGenGpuInputId();

void xprGpuProgramBindInput(XprGpuProgram* self, size_t gpuInputId, XprGpuProgramInput* inputs, size_t count);

typedef enum XprGpuDrawFlag
{
	XprGpuDraw_Stripped		= 0x0001,	//!< draw primitive strips, not supported with point and patch.

	XprGpuDraw_Indexed		= 0x0010,	//!< draw with a 16-bit index buffer
	XprGpuDraw_Indexed8		= 0x0011,	//!< draw with a 8-bit index buffer
	XprGpuDraw_Indexed32	= 0x0012,	//!< draw with a 32-bit index buffer
	
} XprGpuDrawFlag;

void xprGpuDrawPoint(size_t offset, size_t count);

void xprGpuDrawLine(size_t offset, size_t count, size_t flags);
void xprGpuDrawLineIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags);

void xprGpuDrawTriangle(size_t offset, size_t count, size_t flags);
void xprGpuDrawTriangleIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags);

void xprGpuDrawPatch(size_t offset, size_t count, size_t vertexPerPatch, size_t flags);
void xprGpuDrawPatchIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t vertexPerPatch, size_t flags);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__