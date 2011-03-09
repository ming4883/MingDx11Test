#include "RenderTarget.d3d9.h"

XprRenderTarget* xprRenderTargetAlloc()
{
	XprRenderTargetImpl* self = malloc(sizeof(XprRenderTargetImpl));
	memset(self, 0, sizeof(XprRenderTargetImpl));
	return &self->i;
}

void xprRenderTargetFree(XprRenderTarget* self)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	if(self->flags & XprRenderTarget_Inited) {
		
		XprRenderBufferImpl* it; XprRenderBufferImpl* tmp;

		LL_FOREACH_SAFE(impl->bufferList, it, tmp) {
			LL_DELETE(impl->bufferList, it);
			xprTextureFree(it->i.texture);
			IDirect3DSurface9_Release(it->d3dsurf);
			free(it);
		}
	}
	free(self);
}

void xprRenderTargetInit(XprRenderTarget* self)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	if(nullptr == self)
		return;

	if(self->flags & XprRenderTarget_Inited) {
		xprDbgStr("XprRenderTarget already inited!\n");
		return;
	}

	self->flags |= XprRenderTarget_Inited;
}

D3DFORMAT xprD3D9_DEPTH_FORMAT[] = {
	D3DFMT_D16,
	D3DFMT_D32,
	D3DFMT_D24S8,
};

XprRenderBuffer* xprRenderTargetAcquireBuffer(XprRenderTarget* self, size_t width, size_t height, XprGpuFormat format)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	XprRenderBufferImpl* buffer;
	XprRenderBufferImpl* it;
	LL_FOREACH(impl->bufferList, it) {
		XprTexture* tex = it->i.texture;
		if(XprFalse == it->acquired && (width == tex->width) && (height == tex->height) && (format == tex->format)) {
			return &it->i;
		}
	}

	buffer = malloc(sizeof(XprRenderBufferImpl));
	buffer->acquired = XprTrue;

	if(format & XprGpuFormat_Depth) {
		IDirect3DDevice9_CreateDepthStencilSurface(
			xprAPI.d3ddev,
			width, height,
			xprD3D9_DEPTH_FORMAT[XprGpuFormat_Depth & 0xffff],
			D3DMULTISAMPLE_NONE, 0,
			TRUE, 
			&buffer->d3dsurf,
			nullptr);
	}
	else {
		buffer->i.texture = xprTextureAlloc();
		xprTextureInitRtt(buffer->i.texture, width, height, 0, 1, format);
		IDirect3DTexture9_GetSurfaceLevel(
			((XprTextureImpl*)buffer->i.texture)->d3dtex, 0,
			&buffer->d3dsurf);
	}

	LL_APPEND(impl->bufferList, buffer);
	++impl->bufferCount;

	return &buffer->i;
}

void xprRenderTargetReleaseBuffer(XprRenderTarget* self, XprRenderBuffer* buffer)
{
	if(nullptr == self)
		return;

	if(nullptr == buffer)
		return;

	((XprRenderBufferImpl*)buffer)->acquired = XprFalse;
}

void xprRenderTargetPreRender(XprRenderTarget* self, XprRenderBuffer** colors, XprRenderBuffer* depth)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	size_t bufCnt;
	XprRenderBuffer** curr;
	if(nullptr == self) {
		IDirect3DDevice9_SetRenderTarget(xprAPI.d3ddev, 0, xprAPI.d3dcolorbuf);
		IDirect3DDevice9_SetDepthStencilSurface(xprAPI.d3ddev, xprAPI.d3ddepthbuf);
		return;
	}

	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = (XprRenderBuffer**)colors;
		while(*curr != nullptr) {
			XprRenderBufferImpl* buf = (XprRenderBufferImpl*)*curr;
			IDirect3DDevice9_SetRenderTarget(xprAPI.d3ddev, bufCnt, buf->d3dsurf);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		XprTexture* tex = depth->texture;
		XprRenderBufferImpl* buf = (XprRenderBufferImpl*)depth;
		IDirect3DDevice9_SetDepthStencilSurface(xprAPI.d3ddev, buf->d3dsurf);
	}
	else {
		IDirect3DDevice9_SetDepthStencilSurface(xprAPI.d3ddev, nullptr);
	}
}

void xprRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax)
{
	D3DVIEWPORT9 vp;
	vp.X = (DWORD)x;
	vp.Y = (DWORD)y;
	vp.Width  = (DWORD)w;
	vp.Height = (DWORD)h;
	vp.MinZ = zmin * 0.5f + 0.5f;
	vp.MaxZ = zmax * 0.5f + 0.5f;;

	IDirect3DDevice9_SetViewport(xprAPI.d3ddev, &vp);
}

void xprRenderTargetClearColor(float r, float g, float b, float a)
{
	IDirect3DDevice9_Clear(xprAPI.d3ddev, 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_COLORVALUE(r, g, b, a), 1, 0);
}

void xprRenderTargetClearDepth(float z)
{
	IDirect3DDevice9_Clear(xprAPI.d3ddev, 0, nullptr, D3DCLEAR_ZBUFFER, 0, z, 0);
}