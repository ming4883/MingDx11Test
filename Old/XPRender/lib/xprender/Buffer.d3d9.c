#include "Buffer.d3d9.h"
#include "Memory.h"

XprBuffer* xprBufferAlloc()
{
	XprBufferImpl* self = xprMemory()->alloc(sizeof(XprBufferImpl), "XprBuffer");
	memset(self, 0, sizeof(XprBufferImpl));
	return &self->i;
}

void xprBufferFree(XprBuffer* self)
{
	XprBufferImpl* impl = (XprBufferImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != impl->d3dvb)
		IDirect3DVertexBuffer9_Release(impl->d3dvb);

	if(nullptr != impl->d3dib)
		IDirect3DIndexBuffer9_Release(impl->d3dib);

	xprMemory()->free(self, "XprBuffer");
}

XprBool xprBufferInit(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData)
{
	HRESULT hr;
	XprBufferImpl* impl = (XprBufferImpl*)self;
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	if(XprBufferType_Vertex == type) {
		hr = IDirect3DDevice9_CreateVertexBuffer(xprAPI.d3ddev, self->sizeInBytes, 0, 0, D3DPOOL_DEFAULT, &impl->d3dvb, nullptr);
		if(FAILED(hr)) {
			xprDbgStr("d3d9 failed to create vertex buffer %8x", hr);
			return XprFalse;
		}
	}
	else if(XprBufferType_Index == type) {
		hr = IDirect3DDevice9_CreateIndexBuffer(xprAPI.d3ddev, self->sizeInBytes, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &impl->d3dib, nullptr);
		if(FAILED(hr)) {
			xprDbgStr("d3d9 failed to create index buffer %8x", hr);
			return XprFalse;
		}
	}
	else if(XprBufferType_Index32 == type) {
		hr = IDirect3DDevice9_CreateIndexBuffer(xprAPI.d3ddev, self->sizeInBytes, 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &impl->d3dib, nullptr);
		if(FAILED(hr)) {
			xprDbgStr("d3d9 failed to create index 32 buffer %8x", hr);
			return XprFalse;
		}
	}
	else {
		xprDbgStr("uniform buffer is not supported on d3d9");
		return XprFalse;
	}

	self->flags = XprBuffer_Inited;
	return XprTrue;
}

void xprBufferUpdate(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	HRESULT hr;
	XprBufferImpl* impl = (XprBufferImpl*)self;

	//UINT lockFlags = D3DLOCK_DISCARD;
	UINT lockFlags = 0;

	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	if(nullptr != impl->d3dvb) {
		void* ptr;
		hr = IDirect3DVertexBuffer9_Lock(impl->d3dvb, offsetInBytes, sizeInBytes, &ptr, lockFlags);
		if(FAILED(hr)) {
			return;
		}

		memcpy(ptr, data, sizeInBytes);

		IDirect3DVertexBuffer9_Unlock(impl->d3dvb);
	}

	if(nullptr != impl->d3dib) {
		void* ptr;
		hr = IDirect3DIndexBuffer9_Lock(impl->d3dib, offsetInBytes, sizeInBytes, &ptr, lockFlags);
		if(FAILED(hr)) {
			return;
		}

		memcpy(ptr, data, sizeInBytes);

		IDirect3DVertexBuffer9_Unlock(impl->d3dib);
	}
}

void* xprBufferMap(XprBuffer* self, XprBufferMapAccess access)
{
	HRESULT hr;
	XprBufferImpl* impl = (XprBufferImpl*)self;
	void* ret = nullptr;

	//UINT lockFlags = D3DLOCK_DISCARD;
	UINT lockFlags = 0;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & XprBuffer_Mapped))
		return nullptr;

	if(nullptr != impl->d3dvb) {
		hr = IDirect3DVertexBuffer9_Lock(impl->d3dvb, 0, self->sizeInBytes, &ret, lockFlags);
		if(FAILED(hr)) {
			return nullptr;
		}
	}

	if(nullptr != impl->d3dib) {
		hr = IDirect3DIndexBuffer9_Lock(impl->d3dib, 0, self->sizeInBytes, &ret, lockFlags);
		if(FAILED(hr)) {
			return nullptr;
		}
	}

	self->flags |= XprBuffer_Mapped;

	return ret;
}

void xprBufferUnmap(XprBuffer* self)
{
	XprBufferImpl* impl = (XprBufferImpl*)self;
	if(nullptr == self)
		return;

	if(0 == (self->flags & XprBuffer_Mapped))
		return;

	if(nullptr != impl->d3dvb) {
		IDirect3DVertexBuffer9_Unlock(impl->d3dvb);
	}

	if(nullptr != impl->d3dib) {
		IDirect3DVertexBuffer9_Unlock(impl->d3dib);
	}

	self->flags &= ~XprBuffer_Mapped;
}
