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

XprBool xprBufferInit(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData)
{
	HRESULT hr;
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	if(XprBufferType_Vertex == type) {
		hr = IDirect3DDevice9_CreateVertexBuffer(xprAPI.d3ddev, self->sizeInBytes, D3DUSAGE_DYNAMIC, 0, D3DPOOL_MANAGED, &self->impl->d3dvb, nullptr);
		if(FAILED(hr)) {
			XprDbgStr("d3d9 failed to create vertex buffer %8x", hr);
			return XprFalse;
		}
	}
	else if(XprBufferType_Index == type) {
		hr = IDirect3DDevice9_CreateIndexBuffer(xprAPI.d3ddev, self->sizeInBytes, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_MANAGED, &self->impl->d3dib, nullptr);
		if(FAILED(hr)) {
			XprDbgStr("d3d9 failed to create index buffer %8x", hr);
			return XprFalse;
		}
	}
	else
	{
		XprDbgStr("uniform buffer is not supported on d3d9");
		return XprFalse;
	}

	self->flags = XprBuffer_Inited;
	return XprTrue;
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
