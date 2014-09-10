#include "Shader.gl.h"
#include "Texture.gl.h"
#include "Buffer.gl.h"
#include "Memory.h"
#include <stdio.h>

static GLenum xprGL_SHADER_TYPE[] = {
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
	XprGpuShaderImpl* self = xprMemory()->alloc(sizeof(XprGpuShaderImpl), "XprGpuShader");
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
			char* buf = (char*)xprMemory()->alloc(len, "XprGpuShader");
			glGetShaderInfoLog(impl->glName, len, nullptr, buf);
			xprDbgStr("glCompileShader failed: %s", buf);
			xprMemory()->free(buf, "XprGpuShader");
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
	xprMemory()->free(self, "XprGpuShader");
}

XprGpuProgram* xprGpuProgramAlloc()
{
	XprGpuProgramImpl* self = xprMemory()->alloc(sizeof(XprGpuProgramImpl), "XprGpuProgram");
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
			char* buf = (char*)xprMemory()->alloc(len, "XprGpuProgram");
			glGetProgramInfoLog(impl->glName, len, nullptr, buf);
			xprDbgStr("glLinkProgram failed: %s", buf);
			xprMemory()->free(buf, "XprGpuProgram");
		}
		return XprFalse;
	}

	self->flags |= XprGpuProgram_Inited;

	glUseProgram(impl->glName);

	// query all cache
	{
		GLuint i;
		GLint uniformCnt;
		GLsizei uniformLength;
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[32];
		GLuint texunit = 0;

		glGetProgramiv(impl->glName, GL_ACTIVE_UNIFORMS, &uniformCnt);

		impl->uniforms = xprMemory()->alloc(sizeof(XprGpuProgramUniform) * uniformCnt, "XprGpuProgram");
		memset(impl->uniforms, 0, sizeof(XprGpuProgramUniform) * uniformCnt);
		xprDbgStr("glProgram %d has %d cache\n", impl->glName, uniformCnt);

		for(i=0; i<uniformCnt; ++i) {
			XprGpuProgramUniform* uniform;
			glGetActiveUniform(impl->glName, i, xprCountOf(uniformName), &uniformLength, &uniformSize, &uniformType, uniformName);
			uniform = &impl->uniforms[i];
			uniform->hash = XprHash(uniformName);
			uniform->loc = i;
			uniform->size = uniformSize;
			uniform->texunit = texunit;

			HASH_ADD_INT(impl->cache, hash, uniform);

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

	xprMemory()->free(impl->uniforms, "XprGpuProgram");

	glDeleteProgram(impl->glName);

#if !defined(XPR_GLES_2)
	glDeleteVertexArrays(1, &impl->glVertexArray);
#endif

	xprMemory()->free(self, "XprGpuProgram");
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

	HASH_FIND_INT(impl->cache, &hash, uniform);
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

	HASH_FIND_INT(impl->cache, &hash, uniform);
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

	HASH_FIND_INT(impl->cache, &hash, uniform);
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

	HASH_FIND_INT(impl->cache, &hash, uniform);
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

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;

	glUniformMatrix4fv(uniform->loc, count, transpose, value);
	return XprTrue;
}

static GLenum xprGL_SAMPLER_MIN_FILTER[] = {
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_LINEAR,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST,
	GL_LINEAR,
};

static GLenum xprGL_SAMPLER_MAG_FILTER[] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST,
	GL_LINEAR,
};

static GLenum xprGL_SAMPLER_ADDRESS[] = {
	GL_REPEAT,
	GL_CLAMP_TO_EDGE,
};

XprBool xprGpuProgramUniformTexture(XprGpuProgram* self, XprHashCode hash, struct XprTexture* texture, const struct XprSampler* sampler)
{
	XprGpuProgramUniform* uniform;
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	if(nullptr == self)
		return XprFalse;

	if(0 == (self->flags & XprGpuProgram_Inited)) {
		//xprDbgStr("XprGpuProgram is not inited!\n");
		return XprFalse;
	}

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return XprFalse;

	if(uniform->texunit < 0) {
		xprDbgStr("Not a texture!\n");
		return XprFalse;
	}

	glActiveTexture(GL_TEXTURE0 + uniform->texunit);
	if(nullptr == texture)
		glBindTexture(GL_TEXTURE_2D, 0);
	else {
		int gltarget = ((XprTextureImpl*)texture)->glTarget;
		int glname = ((XprTextureImpl*)texture)->glName;
		glBindTexture(gltarget, glname);
		glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, xprGL_SAMPLER_MAG_FILTER[sampler->filter]);
		glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, xprGL_SAMPLER_MIN_FILTER[sampler->filter]);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, xprGL_SAMPLER_ADDRESS[sampler->addressU]);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, xprGL_SAMPLER_ADDRESS[sampler->addressV]);
#if !defined(XPR_GLES_2)
		if(GL_TEXTURE_2D != gltarget) {
			glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, xprGL_SAMPLER_ADDRESS[sampler->addressW]);
		}
#endif
	}

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
	for(i=0; i<xprCountOf(XprInputGpuFormatMappings); ++i) {
		XprInputGpuFormatMapping* mapping = &XprInputGpuFormatMappings[i];
		if(xprFormat == mapping->xprFormat)
			return mapping;
	}

	return nullptr;
}

size_t xprGenGpuInputId()
{
	return ++xprAPI.gpuInputId;
}

void xprGpuProgramBindInput(XprGpuProgram* self, size_t gpuInputId, XprGpuProgramInput* inputs, size_t count)
{
	XprGpuProgramImpl* impl = (XprGpuProgramImpl*)self;

	size_t attri = 0;

#if !defined(XPR_GLES_2)
	glBindVertexArray(impl->glVertexArray);
#endif

	for(attri=0; attri<count; ++attri) {
		XprGpuProgramInput* i = &inputs[attri];

		if(nullptr == i->buffer)
			continue;

		if(XprBufferType_Index == i->buffer->type) {
			// bind index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((XprBufferImpl*)i->buffer)->glName);
		}
		else if(XprBufferType_Vertex == i->buffer->type) {
			// bind vertex buffer
			XprInputGpuFormatMapping* m = xprInputGpuFormatMappingGet(i->format);

			int loc = glGetAttribLocation(impl->glName, i->name);

			if(nullptr != m && -1 != loc) {
				glBindBuffer(GL_ARRAY_BUFFER, ((XprBufferImpl*)i->buffer)->glName);
				glVertexAttribPointer(loc, m->elemCnt, m->elemType, m->normalized, m->stride, (void*)i->offset);
				glEnableVertexAttribArray(loc);
			}
		}
	}
}

static GLenum xprGL_INDEX_TYPE[] = {
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT,
};

void xprGpuDrawPoint(size_t offset, size_t count)
{
	glDrawArrays(GL_POINTS, offset, count);
}

void xprGpuDrawLine(size_t offset, size_t count, size_t flags)
{
	GLenum mode = (flags & XprGpuDraw_Stripped) ? GL_LINE_STRIP : GL_LINES;
	glDrawArrays(mode, offset, count);
}

void xprGpuDrawLineIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	GLenum mode = (flags & XprGpuDraw_Stripped) ? GL_LINE_STRIP : GL_LINES;
	GLenum indexType = xprGL_INDEX_TYPE[flags & 0x000F];
#if defined(XPR_GLES_2)
	glDrawElements(mode, count, indexType, (void*)offset);
#else
	glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)offset);
#endif
}

void xprGpuDrawTriangle(size_t offset, size_t count, size_t flags)
{
	GLenum mode = (flags & XprGpuDraw_Stripped) ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
	glDrawArrays(mode, offset, count);
}

void xprGpuDrawTriangleIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	GLenum mode = (flags & XprGpuDraw_Stripped) ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
	GLenum indexType = xprGL_INDEX_TYPE[flags & 0x000F];
#if defined(XPR_GLES_2)
	glDrawElements(mode, count, indexType, (void*)offset);
#else
	glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)offset);
#endif
}

void xprGpuDrawPatch(size_t offset, size_t count, size_t vertexPerPatch, size_t flags)
{
#if !defined(XPR_GLES_2)
	GLenum mode = GL_PATCHES;
	if(nullptr != glPatchParameteri) {
		glPatchParameteri(GL_PATCH_VERTICES, vertexPerPatch);
		glDrawArrays(mode, offset, count);
	}
#endif
}

void xprGpuDrawPatchIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t vertexPerPatch, size_t flags)
{
#if !defined(XPR_GLES_2)
	GLenum mode = GL_PATCHES;
	GLenum indexType = xprGL_INDEX_TYPE[flags & 0x000F];
	if(nullptr != glPatchParameteri) {
		glPatchParameteri(GL_PATCH_VERTICES, vertexPerPatch);
		glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)offset);
	}
#endif
}
