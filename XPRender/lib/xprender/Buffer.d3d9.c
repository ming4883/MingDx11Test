#include "Buffer.d3d9.h"

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

	if(nullptr != self->impl->d3dvb)
		IDirect3DVertexBuffer9_Release(self->impl->d3dvb);

	if(nullptr != self->impl->d3dvb)
		IDirect3DIndexBuffer9_Release(self->impl->d3dib);

	free(self);
}

void xprBufferInit(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData)
{
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	/*
	glGenBuffers(1, &self->impl->glName);
	glBindBuffer(xprGL_BUFFER_TARGET[self->type], self->impl->glName);
	glBufferData(xprGL_BUFFER_TARGET[self->type], self->sizeInBytes, initialData, GL_STREAM_DRAW);
	*/

	self->flags = XprBuffer_Inited;
}

void xprBufferUpdate(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	//glBindBuffer(xprGL_BUFFER_TARGET[self->type], self->impl->glName);
	//glBufferSubData(xprGL_BUFFER_TARGET[self->type], offsetInBytes, sizeInBytes, data);
}

void* xprBufferMap(XprBuffer* self, XprBufferMapAccess access)
{
	void* ret = nullptr;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & XprBuffer_Mapped))
		return nullptr;

	return ret;
}

void xprBufferUnmap(XprBuffer* self)
{
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprBuffer_Mapped))
		return;

}
