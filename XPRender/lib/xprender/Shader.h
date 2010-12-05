#ifndef __XPRENDER_SHADER_H__
#define __XPRENDER_SHADER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// XprShader
typedef enum XprShaderType
{
	XprShaderType_Vertex,
	XprShaderType_Geometry,
	XprShaderType_Fragment,
} XprShaderType;

typedef enum XprShaderFlag
{
	XprShaderFlag_Compiled = 1 << 0,
} XprShaderFlag;

typedef struct XprShader
{
	int name;
	XprShaderType type;
	size_t flags;

} XprShader;

XprShader* XprShader_new(const char** sources, size_t srcCnt, XprShaderType type);

void XprShader_free(XprShader* self);

// XprPipeline
typedef enum XprPipelineFlag
{
	XprPipelineFlag_Linked = 1 << 0,
} XprPipelineFlag;

typedef struct XprPipeline
{
	int name;
	size_t flags;

} XprPipeline;

XprPipeline* XprPipeline_new(const XprShader** const shaders, size_t shaderCnt);

void XprPipeline_free(XprPipeline* self);

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_H__