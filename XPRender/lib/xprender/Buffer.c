#include "Buffer.gl.h"

static GLenum xprGL_BUFFER_TARGETS[] = {
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

XprBuffer* XprBuffer_alloc()
{
	XprBuffer* self;
	XprAllocWithImpl(self, XprBuffer, XprBufferImpl);

	return self;
}

void XprBuffer_free(XprBuffer* self)
{
	if(nullptr == self)
		return;

	glDeleteBuffers(1, &self->impl->glName);

	free(self);
}

void XprBuffer_init(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData)
{
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	glGenBuffers(1, &self->impl->glName);
	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->impl->glName);
	glBufferData(xprGL_BUFFER_TARGETS[self->type], self->sizeInBytes, initialData, GL_STREAM_DRAW);

	self->flags = XprBufferFlag_Inited;
}

void XprBuffer_update(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->impl->glName);
	glBufferSubData(xprGL_BUFFER_TARGETS[self->type], offsetInBytes, sizeInBytes, data);
}

void* XprBuffer_map(XprBuffer* self, XprBufferMapAccess access)
{
	void* ret = nullptr;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & XprBufferFlag_Mapped))
		return nullptr;

#if !defined(XPR_GLES_2)
	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->impl->glName);
	ret = glMapBuffer(xprGL_BUFFER_TARGETS[self->type], xprGL_BUFFER_MAP_ACCESS[access]);
	self->flags |= XprBufferFlag_Mapped;

#endif

	return ret;
}

void XprBuffer_unmap(XprBuffer* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprBufferFlag_Mapped))
		return;

#if !defined(XPR_GLES_2)
	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->impl->glName);
	glUnmapBuffer(xprGL_BUFFER_TARGETS[self->type]);

	self->flags &= ~XprBufferFlag_Mapped;
#endif
}
