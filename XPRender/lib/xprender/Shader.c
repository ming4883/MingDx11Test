#include "Shader.h"
#include <stdio.h>
#include <string.h>

#include<GL/glew.h>

GLenum xprGL_SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER,
};

XprGpuShader* XprGpuShader_alloc()
{
	XprGpuShader* self = (XprGpuShader*)malloc(sizeof(XprGpuShader));
	memset(self, 0, sizeof(XprGpuShader));
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
	self->name = glCreateShader(xprGL_SHADER_TYPE[self->type]);
	self->flags = 0;

	glShaderSource(self->name, srcCnt, sources, nullptr);
	glCompileShader(self->name);

	glGetShaderiv(self->name, GL_COMPILE_STATUS, &compileStatus);

	if(GL_FALSE == compileStatus)
	{
		GLint len;
		glGetShaderiv(self->name, GL_INFO_LOG_LENGTH, &len);
		if(len > 0)
		{
			char* buf = (char*)malloc(len);
			glGetShaderInfoLog(self->name, len, nullptr, buf);
			XprDbgStr(buf);
			free(buf);
		}
	}
	else
	{
		self->flags |= XprGpuShaderFlag_Compiled;
	}
}

void XprGpuShader_free(XprGpuShader* self)
{
	if(nullptr == self)
		return;

	glDeleteShader(self->name);
	free(self);
}

XprGpuProgram* XprGpuProgram_alloc()
{
	XprGpuProgram* self = (XprGpuProgram*)malloc(sizeof(XprGpuProgram));
	memset(self, 0, sizeof(XprGpuProgram));
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
	
	self->name = glCreateProgram();
	self->flags = 0;

	// attach shaders
	for(i=0; i<shaderCnt; ++i)
	{
		if(nullptr != shaders[i])
			glAttachShader(self->name, shaders[i]->name);
	}

	// link program
	glLinkProgram(self->name);

	glGetProgramiv(self->name, GL_LINK_STATUS, &linkStatus);
	if(GL_FALSE == linkStatus)
	{
		GLint len;
		glGetProgramiv(self->name, GL_INFO_LOG_LENGTH, &len);
		if(len > 0)
		{
			char* buf = (char*)malloc(len);
			glGetProgramInfoLog(self->name, len, nullptr, buf);
			XprDbgStr(buf);
			free(buf);
		}
	}
	else
	{
		self->flags |= XprGpuProgramFlag_Linked;
	}
}

void XprGpuProgram_free(XprGpuProgram* self)
{
	if(nullptr == self)
		return;

	glDeleteProgram(self->name);
	free(self);
}