#include "Buffer.gl.h"

static GLenum xprGL_BUFFER_TARGET[] = {
	GL_ARRAY_BUFFER,
	GL_ELEMENT_ARRAY_BUFFER,
#if !defined(XPR_GLES_2)
	GL_UNIFORM_BUFFER,
#endif
};

#if !defined(XPR_GLES_2)
static GLenum xprGL_BUFFER_MAP_ACCESS[] = {
	GL_READ_ONLY,
	GL_WRITE_ONLY,
	GL_READ_WRITE,
};
#endif

XprBuffer* xprBufferAlloc()
{
	XprBuffer* self;
	XprAllocWithImpl(self, XprBuffer, XprBufferImpl);

	return self;
}

void xprBufferFree(XprBuffer* self)
{
	if(nullptr == self)
		return;

	glDeleteBuffers(1, &self->impl->glName);

	free(self);
}

XprBool xprBufferInit(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData)
{
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	glGenBuffers(1, &self->impl->glName);
	glBindBuffer(xprGL_BUFFER_TARGET[self->type], self->impl->glName);
	glBufferData(xprGL_BUFFER_TARGET[self->type], self->sizeInBytes, initialData, GL_STREAM_DRAW);

	self->flags = XprBuffer_Inited;

	return XprTrue;
}

void xprBufferUpdate(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	glBindBuffer(xprGL_BUFFER_TARGET[self->type], self->impl->glName);
	glBufferSubData(xprGL_BUFFER_TARGET[self->type], offsetInBytes, sizeInBytes, data);
}

void* xprBufferMap(XprBuffer* self, XprBufferMapAccess access)
{
	void* ret = nullptr;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & XprBuffer_Mapped))
		return nullptr;

#if !defined(XPR_GLES_2)
	glBindBuffer(xprGL_BUFFER_TARGET[self->type], self->impl->glName);
	ret = glMapBuffer(xprGL_BUFFER_TARGET[self->type], xprGL_BUFFER_MAP_ACCESS[access]);
	self->flags |= XprBuffer_Mapped;

#endif

	return ret;
}

void xprBufferUnmap(XprBuffer* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprBuffer_Mapped))
		return;

#if !defined(XPR_GLES_2)
	glBindBuffer(xprGL_BUFFER_TARGET[self->type], self->impl->glName);
	glUnmapBuffer(xprGL_BUFFER_TARGET[self->type]);

	self->flags &= ~XprBuffer_Mapped;
#endif
}
