#include "RenderTarget.gl3.h"

XprRenderTarget* XprRenderTarget_alloc()
{
	XprRenderTarget* self;
	XprAllocWithImpl(self, XprRenderTarget, XprRenderTargetImpl);

	return self;
}

void XprRenderTarget_free(XprRenderTarget* self)
{
	if(self->flags & XprRenderTargetFlag_Inited) {
		glDeleteFramebuffers(1, &self->impl->glName);
	}
	free(self);
}

void XprRenderTarget_init(XprRenderTarget* self, size_t width, size_t height)
{
	if(nullptr == self)
		return;

	if(self->flags & XprRenderTargetFlag_Inited) {
		XprDbgStr("XprRenderTarget already inited!\n");
		return;
	}

	self->width = width;
	self->height = height;

	glGenFramebuffers(1, &self->impl->glName);

	self->flags |= XprRenderTargetFlag_Inited;
}