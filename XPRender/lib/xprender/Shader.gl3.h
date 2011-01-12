#ifndef __XPRENDER_SHADER_GL3_H__
#define __XPRENDER_SHADER_GL3_H__

#include "Shader.h"
#include "uthash/uthash.h"

#include<GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuShaderImpl
{
	int glName;

} XprGpuShaderImpl;

typedef struct XprGpuProgramUniformBinding
{
	GLuint hash;
	GLuint loc;
	GLuint texunit;
	UT_hash_handle hh;
} XprGpuProgramSamplerBinding;

typedef struct XprGpuProgramImpl
{
	int glName;

} XprGpuProgramImpl;

#ifdef __cplusplus
}
#endif

#endif // __XPRENDER_SHADER_GL3_H__