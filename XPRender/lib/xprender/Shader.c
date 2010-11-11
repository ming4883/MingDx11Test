#include "Shader.h"
#include <stdio.h>

#include<GL/glew.h>

GLenum xprGL_SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER,
};

xprShader* xprShader_new(const char** sources, size_t srcCnt, xprShaderType type)
{
	int compileStatus;

	xprShader* self = (xprShader*)malloc(sizeof(xprShader));

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
			printf(buf);
			free(buf);
		}
	}

	return self;
}

void xprShader_free(xprShader* self)
{
	glDeleteShader(self->name);
	free(self);
}