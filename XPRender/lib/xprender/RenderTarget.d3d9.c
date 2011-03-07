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
			free(it);
		}

		//glDeleteFramebuffers(1, &impl->glName);
	}
	free(self);
}

void xprRenderTargetInit(XprRenderTarget* self, size_t width, size_t height)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	if(nullptr == self)
		return;

	if(self->flags & XprRenderTarget_Inited) {
		xprDbgStr("XprRenderTarget already inited!\n");
		return;
	}

	self->width = width;
	self->height = height;

	//glGenFramebuffers(1, &impl->glName);
	
	self->flags |= XprRenderTarget_Inited;
}

XprRenderBuffer* xprRenderTargetAcquireBuffer(XprRenderTarget* self, XprGpuFormat format)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	XprRenderBufferImpl* buffer;
	XprRenderBufferImpl* it;
	LL_FOREACH(impl->bufferList, it) {
		if(XprFalse == it->acquired && (it->i.texture->format == format)) {
			return &it->i;
		}
	}

	buffer = malloc(sizeof(XprRenderBufferImpl));
	buffer->acquired = XprTrue;
	buffer->i.texture = xprTextureAlloc();
	xprTextureInitRtt(buffer->i.texture, self->width, self->height, 0, 1, format);

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
		return;
	}

	//glBindFramebuffer(GL_FRAMEBUFFER, impl->glName);
	
	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = (XprRenderBuffer**)colors;
		while(*curr != nullptr) {
			XprTexture* tex = (*curr)->texture;
			//glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachmentPoints[bufCnt], tex->impl->glTarget, tex->impl->glName, 0);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		XprTexture* tex = depth->texture;
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->impl->glTarget, tex->impl->glName, 0);
	}

	// assign buffer bindings
	//glDrawBuffers(bufCnt, glAttachmentPoints);
}

void xprRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax)
{
	D3DVIEWPORT9 vp;
	vp.X = (DWORD)x;
	vp.Y = (DWORD)y;
	vp.Width  = (DWORD)w;
	vp.Height = (DWORD)h;
	vp.MinZ = zmin;
	vp.MaxZ = zmax;

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