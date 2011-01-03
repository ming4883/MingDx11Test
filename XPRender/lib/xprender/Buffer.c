#include "Buffer.h"
#include <GL/glew.h>
#include <string.h>

static GLenum xprGL_BUFFER_TARGETS[] = {
	GL_ARRAY_BUFFER,
	GL_ELEMENT_ARRAY_BUFFER,
	GL_UNIFORM_BUFFER,
};

static GLenum xprGL_BUFFER_MAP_ACCESS[] = {
	GL_READ_ONLY,
	GL_WRITE_ONLY,
	GL_READ_WRITE,
};

XprBuffer* XprBuffer_alloc()
{
	XprBuffer* self = (XprBuffer*)malloc(sizeof(XprBuffer));
	memset(self, 0, sizeof(XprBuffer));
	return self;
}

void XprBuffer_init(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData)
{
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	glGenBuffers(1, &self->name);
	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);
	glBufferData(xprGL_BUFFER_TARGETS[self->type], self->sizeInBytes, initialData, GL_STREAM_DRAW);

	self->flags = XprBufferFlag_Inited;
}


void XprBuffer_free(XprBuffer* self)
{
	if(nullptr == self)
		return;

	glDeleteBuffers(1, &self->name);

	free(self);
}
void XprBuffer_update(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);
	glBufferSubData(xprGL_BUFFER_TARGETS[self->type], offsetInBytes, sizeInBytes, data);
}

void* XprBuffer_map(XprBuffer* self, XprBufferMapAccess access)
{
	void* ret = nullptr;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & XprBufferFlag_Mapped))
		return nullptr;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);

	ret = glMapBuffer(xprGL_BUFFER_TARGETS[self->type], xprGL_BUFFER_MAP_ACCESS[access]);

	self->flags |= XprBufferFlag_Mapped;

	return ret;
}

void XprBuffer_unmap(XprBuffer* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprBufferFlag_Mapped))
		return;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);
	glUnmapBuffer(xprGL_BUFFER_TARGETS[self->type]);

	self->flags &= ~XprBufferFlag_Mapped;
}
