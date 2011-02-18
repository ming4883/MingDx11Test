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

void xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type)
{
	int compileStatus;

	if(self->flags & XprGpuShaderFlag_Compiled) {
		XprDbgStr("XprGpuShader already inited!\n");
		return;
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
	}
	else {
		self->flags |= XprGpuShaderFlag_Compiled;
	}
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

void xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt)
{
	size_t i;
	int linkStatus;

	if(self->flags & XprGpuProgramFlag_Linked) {
		XprDbgStr("XprGpuProgram already inited!\n");
		return;
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
		return;
	}
	
	self->flags |= XprGpuProgramFlag_Linked;

#if !defined(XPR_GLES_2)
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
				case GL_SAMPLER_1D:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_1D_SHADOW:
				case GL_SAMPLER_2D_SHADOW: 
					{	// bind sampler to the specific texture unit
						glUniform1i(i, texunit++);
					}
					break;
				default:
					uniform->texunit = -1;
					break;
			}
			//XprDbgStr("%s %d %d %d\n", uniformName, uniformSize, uniformType, uniform->texunit);
		}
		
	}
#endif
	
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

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	glUseProgram(self->impl->glName);
}

void xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr != uniform) {
		glUniform1fv(uniform->loc, count, value);
	}
}

void xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr != uniform) {
		glUniform2fv(uniform->loc, count, value);
	}
}

void xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr != uniform) {
		glUniform3fv(uniform->loc, count, value);
	}
}

void xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr != uniform) {
		glUniform4fv(uniform->loc, count, value);
	}
}

void xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr != uniform) {
		glUniformMatrix4fv(uniform->loc, count, transpose, value);
	}
}

void xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture)
{
	XprGpuProgramUniform* uniform;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	HASH_FIND_INT(self->impl->uniforms, &hash, uniform);
	if(nullptr != uniform) {
		if(uniform->texunit < 0) {
			XprDbgStr("Not a texture!\n");
			return;
		}
		glActiveTexture(GL_TEXTURE0 + uniform->texunit);
		if(nullptr == texture)
			glBindTexture(GL_TEXTURE_2D, 0);
		else
			glBindTexture(texture->impl->glTarget, texture->impl->glName);
	}
}