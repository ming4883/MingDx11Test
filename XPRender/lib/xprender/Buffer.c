#include "Buffer.h"
#include <GL/glew.h>

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

xprBuffer* xprBuffer_new(xprBufferType type, size_t sizeInBytes, void* initialData)
{
	xprBuffer* self = (xprBuffer*)malloc(sizeof(xprBuffer));

	self->sizeInBytes = sizeInBytes;
	self->type = type;
	self->flags = 0;

	glGenBuffers(1, &self->name);
	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);
	glBufferData(xprGL_BUFFER_TARGETS[self->type], self->sizeInBytes, initialData, GL_STREAM_DRAW);

	return self;
}

void xprBuffer_free(xprBuffer* self)
{
	if(nullptr == self)
		return;

	glDeleteBuffers(1, &self->name);

	free(self);
}
void xprBuffer_update(xprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);
	glBufferSubData(xprGL_BUFFER_TARGETS[self->type], offsetInBytes, sizeInBytes, data);
}

void* xprBuffer_map(xprBuffer* self, xprBufferMapAccess access)
{
	void* ret = nullptr;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & xprBufferFlag_Mapped))
		return nullptr;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);

	ret = glMapBuffer(xprGL_BUFFER_TARGETS[self->type], xprGL_BUFFER_MAP_ACCESS[access]);

	self->flags |= xprBufferFlag_Mapped;

	return ret;
}

void xprBuffer_unmap(xprBuffer* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & xprBufferFlag_Mapped))
		return;

	glBindBuffer(xprGL_BUFFER_TARGETS[self->type], self->name);
	glUnmapBuffer(xprGL_BUFFER_TARGETS[self->type]);

	self->flags &= ~xprBufferFlag_Mapped;
}
