#include "Shader.h"
#include <stdio.h>

#include<GL/glew.h>

GLenum xprGL_SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER,
};

XprShader* XprShader_new(const char** sources, size_t srcCnt, XprShaderType type)
{
	int compileStatus;

	XprShader* self = (XprShader*)malloc(sizeof(XprShader));

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
		self->flags |= XprShaderFlag_Compiled;
	}

	return self;
}

void XprShader_free(XprShader* self)
{
	if(nullptr == self)
		return;

	glDeleteShader(self->name);
	free(self);
}

XprPipeline* XprPipeline_new(const XprShader** const shaders, size_t shaderCnt)
{
	size_t i;
	int linkStatus;
	XprPipeline* self = (XprPipeline*)malloc(sizeof(XprPipeline));

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
		self->flags |= XprPipelineFlag_Linked;
	}

	return self;
}

void XprPipeline_free(XprPipeline* self)
{
	if(nullptr == self)
		return;

	glDeleteProgram(self->name);
	free(self);
}