#ifndef __XPRENDER_SHADER_GL_H__
#define __XPRENDER_SHADER_GL_H__

#include "Opengl.h"
#include "Shader.h"
#include "uthash/uthash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuShaderImpl
{
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
	int glName;
	struct XprGpuProgramUniform* uniforms;

} XprGpuProgramImpl;

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_GL_H__