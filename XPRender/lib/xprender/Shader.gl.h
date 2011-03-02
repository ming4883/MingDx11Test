#ifndef __XPRENDER_SHADER_GL_H__
#define __XPRENDER_SHADER_GL_H__

#include "API.gl.h"
#include "Shader.h"
#include "uthash/uthash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuShaderImpl
{
	XprGpuShader i;

	int glName;

} XprGpuShaderImpl;

typedef struct XprGpuProgramUniform
{
	XprHashCode hash;
	GLuint loc;
	GLuint size;
	int texunit;
	UT_hash_handle hh;

} XprGpuProgramUniform;

typedef struct XprGpuProgramImpl
{
	XprGpuProgram i;

	int glName;
	int glVertexArray;
	struct XprGpuProgramUniform* uniforms;

} XprGpuProgramImpl;

typedef struct XprInputGpuFormatMapping
{
	XprGpuFormat xprFormat;
	int elemCnt;
	int elemType;
	int normalized;
	int stride;
} XprInputGpuFormatMapping;

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_GL_H__