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
	}
	else {
		self->flags |= XprGpuProgramFlag_Linked;
	}
}

void XprGpuProgram_free(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	glDeleteProgram(self->impl->glName);
	free(self);
}