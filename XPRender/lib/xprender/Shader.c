#include "Shader.gl.h"
#include "Texture.gl.h"
#include <stdio.h>

GLenum xprGL_SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
#if !defined(XPR_GLES_2)
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
#endif
};

XprGpuShader* xprGpuShaderAlloc()
{
	XprGpuShader* self;
	XprAllocWithImpl(self, XprGpuShader, XprGpuShaderImpl);

	return self;
}

XprBool xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type)
{
	int compileStatus;

	if(self->flags & XprGpuShader_Inited) {
		XprDbgStr("XprGpuShader already inited!\n");
		return XprFalse;
	}

	self->type = type;
	self->impl->glName = glCreateShader(xprGL_SHADER_TYPE[self->type]);
	self->flags = 0;

	glShaderSource(self->impl->glName, srcCnt, sources, nullptr);
	glCompileShader(self->impl->glName);

	glGetShaderiv(self->impl->glName, GL_COMPILE_STATUS, &compileStatus);

	if(GL_FALSE == compileStatus) {
		GLint len;
		glGetShaderiv(self->impl->glName, GL_INFO_LOG_LENGTH, &len);
		if(len > 0) {
			char* buf = (char*)malloc(len);
			glGetShaderInfoLog(self->impl->glName, len, nullptr, buf);
			XprDbgStr("glCompileShader failed: %s", buf);
			free(buf);
		}

		return XprFalse;
	}
	else {
		self->flags |= XprGpuShader_Inited;
	}

	return XprTrue;
}

void xprGpuShaderFree(XprGpuShader* self)
{
	if(nullptr == self)
		return;

	glDeleteShader(self->impl->glName);
	free(self);
}

XprGpuProgram* xprGpuProgramAlloc()
{
	XprGpuProgram* self;
	XprAllocWithImpl(self, XprGpuProgram, XprGpuProgramImpl);

	return self;
}

XprBool xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt)
{
	size_t i;
	int linkStatus;

	if(self->flags & XprGpuProgram_Inited) {
		XprDbgStr("XprGpuProgram already inited!\n");
		return XprFalse;
	}
	
	self->impl->glName = glCreateProgram();
	self->flags = 0;

	// attach shaders
	for(i=0; i<shaderCnt; ++i) {
		if(nullptr != shaders[i]) {
			glAttachShader(self->impl->glName, shaders[i]->impl->glName);
		}
	}

	// link program
	glLinkProgram(self->impl->glName);

	glGetProgramiv(self->impl->glName, GL_LINK_STATUS, &linkStatus);
	if(GL_FALSE == linkStatus) {
		GLint len;
		glGetProgramiv(self->impl->glName, GL_INFO_LOG_LENGTH, &len);
		if(len > 0) {
			char* buf = (char*)malloc(len);
			glGetProgramInfoLog(self->impl->glName, len, nullptr, buf);
			XprDbgStr("glLinkProgram failed: %s", buf);
			free(buf);
		}
		return XprFalse;
	}
	
	self->flags |= XprGpuProgram_Inited;

	glUseProgram(self->impl->glName);
	
	// query all uniforms
	{
		GLuint i;
		GLuint uniformCnt;
		GLsizei uniformLength;
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[32];
		GLuint texunit = 0;	
		
		glGetProgramiv(self->impl->glName, GL_ACTIVE_UNIFORMS, &uniformCnt);
		
		XprDbgStr("glProgram %d has %d uniforms\n", self->impl->glName, uniformCnt);

		for(i=0; i<uniformCnt; ++i) {
			XprGpuProgramUniform* uniform;
			glGetActiveUniform(self->impl->glName, i, XprCountOf(uniformName), &uniformLength, &uniformSize, &uniformType, uniformName);
			uniform = malloc(sizeof(XprGpuProgramUniform));
			uniform->hash = XprHash(uniformName);
			uniform->loc = i;
			uniform->size = uniformSize;
			uniform->texunit = texunit;

			HASH_ADD_INT(self->impl->uniforms, hash, uniform);
			
			switch(uniformType) {
				case GL_SAMPLER_2D:
				case GL_SAMPLER_CUBE:
#if !defined(XPR_GLES_2)
				case GL_SAMPLER_1D:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_1D_SHADOW:
				case GL_SAMPLER_2D_SHADOW: 
#endif
					{	// bind sampler to the specific texture unit
						glUniform1i(i, texunit++);
					}
					break;
				default:
					uniform->texunit = -1;
					break;
			}
			//XprDbgStr("%s %d %d %d %d\n", uniformName, i, uniformSize, uniformType, uniform->texunit);
		}
		
	}

	return XprTrue;
	
}

void xprGpuProgramFree(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	{
		XprGpuProgramUniform* curr, *temp;
		HASH_ITER(hh, self->impl->uniforms, curr, temp) {
			HASH_DEL(self->impl->uniforms, curr);
			free(curr);
		}
	}

	glDeleteProgram(self->impl->glName);
	free(self);
}

void xprGpuProgramPreRender(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	glUseProgram(self->impl->glName);
}

XprBool xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform1fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform2fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform3fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform4fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniformMatrix4fv(uniform->loc, count, transpose, value);
	return XprTrue;
}

XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//XprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
		
	if(uniform->texunit < 0) {
		XprDbgStr("Not a texture!\n");
		return XprFalse;
	}
	
	glActiveTexture(GL_TEXTURE0 + uniform->texunit);
	if(nullptr == texture)
		glBindTexture(GL_TEXTURE_2D, 0);
	else
		glBindTexture(texture->impl->glTarget, texture->impl->glName);
		
	return XprTrue;
}