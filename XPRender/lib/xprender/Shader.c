#include "Shader.GL3.h"
#include <stdio.h>

GLenum xprGL_SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER,
};

XprGpuShader* XprGpuShader_alloc()
{
	XprGpuShader* self;
	XprAllocWithImpl(self, XprGpuShader, XprGpuShaderImpl);

	return self;
}

void XprGpuShader_init(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type)
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
			XprDbgStr(buf);
			free(buf);
		}
	}
	else {
		self->flags |= XprGpuShaderFlag_Compiled;
	}
}

void XprGpuShader_free(XprGpuShader* self)
{
	if(nullptr == self)
		return;

	glDeleteShader(self->impl->glName);
	free(self);
}

XprGpuProgram* XprGpuProgram_alloc()
{
	XprGpuProgram* self;
	XprAllocWithImpl(self, XprGpuProgram, XprGpuProgramImpl);

	return self;
}

void XprGpuProgram_init(XprGpuProgram* self, const XprGpuShader** const shaders, size_t shaderCnt)
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
			XprDbgStr(buf);
			free(buf);
		}
		return;
	}
	
	self->flags |= XprGpuProgramFlag_Linked;
	{
		GLuint i;
		GLuint uniformCnt;
		GLsizei uniformLength;
		GLint uniformSize;
		GLenum uniformType;
		GLchar uniformName[64];
		glGetProgramiv(self->impl->glName, GL_ACTIVE_UNIFORMS, &uniformCnt);

		for(i=0; i<uniformCnt; ++i) {
			glGetActiveUniform(self->impl->glName, i, 64, &uniformLength, &uniformSize, &uniformType, uniformName);
			XprDbgStr("%s %d %d\n", uniformName, uniformSize, uniformType);
		}
	}
	
}

void XprGpuProgram_free(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	glDeleteProgram(self->impl->glName);
	free(self);
}

void XprGpuProgram_preRender(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	glUseProgram(self->impl->glName);
}

void XprGpuProgram_uniform1fv(XprGpuProgram* self, const char* name, size_t count, const float* value)
{
	int loc;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	loc = glGetUniformLocation(self->impl->glName, name);
	if(loc >= 0) {
		glUniform1fv(loc, count, value);
	}
}

void XprGpuProgram_uniform2fv(XprGpuProgram* self, const char* name, size_t count, const float* value)
{
	int loc;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	loc = glGetUniformLocation(self->impl->glName, name);
	if(loc >= 0) {
		glUniform2fv(loc, count, value);
	}
}

void XprGpuProgram_uniform3fv(XprGpuProgram* self, const char* name, size_t count, const float* value)
{
	int loc;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	loc = glGetUniformLocation(self->impl->glName, name);
	if(loc >= 0) {
		glUniform3fv(loc, count, value);
	}
}

void XprGpuProgram_uniform4fv(XprGpuProgram* self, const char* name, size_t count, const float* value)
{
	int loc;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	loc = glGetUniformLocation(self->impl->glName, name);
	if(loc >= 0) {
		glUniform4fv(loc, count, value);
	}
}

void XprGpuProgram_uniformMtx4fv(XprGpuProgram* self, const char* name, size_t count, XprBool transpose, const float* value)
{
	int loc;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgramFlag_Linked)) {
		XprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	loc = glGetUniformLocation(self->impl->glName, name);
	if(loc >= 0) {
		glUniformMatrix4fv(loc, count, transpose, value);
	}
}
