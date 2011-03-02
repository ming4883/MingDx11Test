#include "Shader.gl.h"
#include "Texture.gl.h"
#include "Buffer.gl.h"
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
	XprGpuShaderImpl* self = malloc(sizeof(XprGpuShaderImpl));
	memset(self, 0, sizeof(XprGpuShaderImpl));
	return &self->i;
}

XprBool xprGpuShaderInit(XprGpuShader* self, const char** sources, size_t srcCnt, XprGpuShaderType type)
{
	int compileStatus;
	XprGpuShaderImpl* impl = (XprGpuShaderImpl*)self;

	if(self->flags & XprGpuShader_Inited) {
		xprDbgStr("XprGpuShader already inited!\n");
		return XprFalse;
	}

	self->flags = 0;
	self->type = type;
	impl->glName = glCreateShader(xprGL_SHADER_TYPE[self->type]);

	glShaderSource(impl->glName, srcCnt, sources, nullptr);
	glCompileShader(impl->glName);

	glGetShaderiv(impl->glName, GL_COMPILE_STATUS, &compileStatus);

	if(GL_FALSE == compileStatus) {
		GLint len;
		glGetShaderiv(impl->glName, GL_INFO_LOG_LENGTH, &len);
		if(len > 0) {
			char* buf = (char*)malloc(len);
			glGetShaderInfoLog(impl->glName, len, nullptr, buf);
			xprDbgStr("glCompileShader failed: %s", buf);
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
	XprGpuShaderImpl* impl = (XprGpuShaderImpl*)self;

	if(nullptr == self)
		return;

	glDeleteShader(impl->glName);
	free(self);
}

XprGpuProgram* xprGpuProgramAlloc()
{
	XprGpuProgramImpl* self = malloc(sizeof(XprGpuProgramImpl));
	memset(self, 0, sizeof(XprGpuProgramImpl));
	return &self->i;
}

XprBool xprGpuProgramInit(XprGpuProgram* self, XprGpuShader** shaders, size_t shaderCnt)
{
	size_t i;
	int linkStatus;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(self->flags & XprGpuProgram_Inited) {
		xprDbgStr("XprGpuProgram already inited!\n");
		return XprFalse;
	}
	
	impl->glName = glCreateProgram();

	// attach shaders
	for(i=0; i<shaderCnt; ++i) {
		if(nullptr != shaders[i]) {
			glAttachShader(impl->glName, ((XprGpuShaderImpl*)shaders[i])->glName);
		}
	}

	// link program
	glLinkProgram(impl->glName);

	glGetProgramiv(impl->glName, GL_LINK_STATUS, &linkStatus);
	if(GL_FALSE == linkStatus) {
		GLint len;
		glGetProgramiv(impl->glName, GL_INFO_LOG_LENGTH, &len);
		if(len > 0) {
			char* buf = (char*)malloc(len);
			glGetProgramInfoLog(impl->glName, len, nullptr, buf);
			xprDbgStr("glLinkProgram failed: %s", buf);
			free(buf);
		}
		return XprFalse;
	}
	
	self->flags |= XprGpuProgram_Inited;

	glUseProgram(impl->glName);
	
	// query all uniforms
	{
		GLuint i;
		GLuint uniformCnt;
		GLsizei uniformLength;
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[32];
		GLuint texunit = 0;	
		
		glGetProgramiv(impl->glName, GL_ACTIVE_UNIFORMS, &uniformCnt);
		
		xprDbgStr("glProgram %d has %d uniforms\n", impl->glName, uniformCnt);

		for(i=0; i<uniformCnt; ++i) {
			XprGpuProgramUniform* uniform;
			glGetActiveUniform(impl->glName, i, XprCountOf(uniformName), &uniformLength, &uniformSize, &uniformType, uniformName);
			uniform = malloc(sizeof(XprGpuProgramUniform));
			uniform->hash = XprHash(uniformName);
			uniform->loc = i;
			uniform->size = uniformSize;
			uniform->texunit = texunit;

			HASH_ADD_INT(impl->uniforms, hash, uniform);
			
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
			//xprDbgStr("%s %d %d %d %d\n", uniformName, i, uniformSize, uniformType, uniform->texunit);
		}
		
	}

#if !defined(XPR_GLES_2)
	glGenVertexArrays(1, &impl->glVertexArray);
#endif

	return XprTrue;
	
}

void xprGpuProgramFree(XprGpuProgram* self)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;
	if(nullptr == self)
		return;

	{
		XprGpuProgramUniform* curr, *temp;
		HASH_ITER(hh, impl->uniforms, curr, temp) {
			HASH_DEL(impl->uniforms, curr);
			free(curr);
		}
	}

	glDeleteProgram(impl->glName);

#if !defined(XPR_GLES_2)
	glDeleteVertexArrays(1, &impl->glVertexArray);
#endif

	free(self);
}

void xprGpuProgramPreRender(XprGpuProgram* self)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return;
	}

	glUseProgram(impl->glName);
}

XprBool xprGpuProgramUniform1fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform1fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniform2fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform2fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniform3fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform3fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniform4fv(XprGpuProgram* self, XprHashCode hash, size_t count, const float* value)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniform4fv(uniform->loc, count, value);
	return XprTrue;
}

XprBool xprGpuProgramUniformMtx4fv(XprGpuProgram* self, XprHashCode hash, size_t count, XprBool transpose, const float* value)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
	
	glUniformMatrix4fv(uniform->loc, count, transpose, value);
	return XprTrue;
}

XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->uniforms, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;
		
	if(uniform->texunit < 0) {
		xprDbgStr("Not a texture!\n");
		return XprFalse;
	}
	
	glActiveTexture(GL_TEXTURE0 + uniform->texunit);
	if(nullptr == texture)
		glBindTexture(GL_TEXTURE_2D, 0);
	else
		glBindTexture(texture->impl->glTarget, texture->impl->glName);
		
	return XprTrue;
}

XprInputGpuFormatMapping XprInputGpuFormatMappings[] = {
	{XprGpuFormat_FloatR32,				1, GL_FLOAT, 0, sizeof(float)},
	{XprGpuFormat_FloatR32G32,			2, GL_FLOAT, 0, sizeof(float) * 2},
	{XprGpuFormat_FloatR32G32B32,		3, GL_FLOAT, 0, sizeof(float) * 3},
	{XprGpuFormat_FloatR32G32B32A32,	4, GL_FLOAT, 0, sizeof(float) * 4},
};

XprInputGpuFormatMapping* xprInputGpuFormatMappingGet(XprGpuFormat xprFormat)
{
	size_t i=0;
	for(i=0; i<XprCountOf(XprInputGpuFormatMappings); ++i) {
		XprInputGpuFormatMapping* mapping = &XprInputGpuFormatMappings[i];
		if(xprFormat == mapping->xprFormat)
			return mapping;
	}

	return nullptr;
}

void xprGpuProgramBindInput(XprGpuProgram* self, XprGpuProgramInput* inputs, size_t count)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	size_t attri = 0;
	
#if !defined(XPR_GLES_2)
	glBindVertexArray(impl->glVertexArray);
#endif

	for(attri=0; attri<count; ++attri) {
		XprGpuProgramInput* i = &inputs[attri];
		XprInputGpuFormatMapping* m = xprInputGpuFormatMappingGet(i->format);

		int loc = glGetAttribLocation(impl->glName, i->name);

		if(nullptr != m && nullptr != i->buffer && -1 != loc) {
			glBindBuffer(GL_ARRAY_BUFFER, i->buffer->impl->glName);
			glVertexAttribPointer(loc, m->elemCnt, m->elemType, m->normalized, m->stride, (void*)i->offset);
			glEnableVertexAttribArray(loc);
		}
	}
}