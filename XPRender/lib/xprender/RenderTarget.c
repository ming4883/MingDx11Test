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
		
		XprRenderBuffer* it; XprRenderBuffer* tmp;

		LL_FOREACH_SAFE(self->impl->bufferList, it, tmp) {
			LL_DELETE(self->impl->bufferList, it);
			XprTexture_free(it->texture);
			free(it);
		}

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

struct XprRenderBuffer* XprRenderTarget_acquireBuffer(XprRenderTarget* self, const char* format)
{
	XprRenderBuffer* buffer;
	XprRenderBuffer* it;
	LL_FOREACH(self->impl->bufferList, it) {
		if(XprFalse == it->acquired && strcmp(it->texture->format, format) == 0) {
			return it;
		}
	}

	buffer = malloc(sizeof(XprRenderBuffer));
	buffer->acquired = XprTrue;
	buffer->texture = XprTexture_alloc();
	XprTexture_initRtt(buffer->texture, self->width, self->height, 0, 1, format);

	LL_APPEND(self->impl->bufferList, buffer);
	++self->impl->bufferCount;

	return buffer;
}

void XprRenderTarget_releaseBuffer(XprRenderTarget* self, struct XprRenderBuffer* buffer)
{
	if(nullptr == self)
		return;

	if(nullptr == buffer)
		return;

	buffer->acquired = XprFalse;
}

struct XprTexture* XprRenderTarget_getTexture(XprRenderTarget* self, struct XprRenderBuffer* buffer)
{
	if(nullptr == self)
		return nullptr;

	if(nullptr == buffer)
		return nullptr;

	return buffer->texture;
}

GLenum glAttachmentPoints[] =
{	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7,
};

void XprRenderTarget_preRender(XprRenderTarget* self, struct XprRenderBuffer** colors, struct XprRenderBuffer* depth)
{
	size_t bufCnt;
	XprRenderBuffer** curr;
	if(nullptr == self) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		return;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, self->impl->glName);
	
	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = colors;
		while(*curr != nullptr) {
			XprTexture* tex = (*curr)->texture;
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, glAttachmentPoints[bufCnt], tex->impl->glTarget, tex->impl->glName, 0);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		XprTexture* tex = depth->texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->impl->glTarget, tex->impl->glName, 0);
	}

	// assign buffer bindings
	glDrawBuffers(bufCnt, glAttachmentPoints);

	{	// check for framebuffer's complete status
		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if(GL_FRAMEBUFFER_COMPLETE != status) {
			XprDbgStr("imcomplete framebuffer status:%x", status);
		}
	}
}