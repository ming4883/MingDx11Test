#include "RenderTarget.gl.h"

XprRenderTarget* xprRenderTargetAlloc()
{
	XprRenderTarget* self;
	XprAllocWithImpl(self, XprRenderTarget, XprRenderTargetImpl);

	return self;
}

void xprRenderTargetFree(XprRenderTarget* self)
{
	if(self->flags & XprRenderTargetFlag_Inited) {
		
		XprRenderBuffer* it; XprRenderBuffer* tmp;

		LL_FOREACH_SAFE(self->impl->bufferList, it, tmp) {
			LL_DELETE(self->impl->bufferList, it);
			xprTextureFree(it->texture);
			free(it);
		}

		glDeleteFramebuffers(1, &self->impl->glName);
	}
	free(self);
}

void xprRenderTargetInit(XprRenderTarget* self, size_t width, size_t height)
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

XprRenderBufferHandle xprRenderTargetAcquireBuffer(XprRenderTarget* self, const char* format)
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
	buffer->texture = xprTextureAlloc();
	xprTextureInitRtt(buffer->texture, self->width, self->height, 0, 1, format);

	LL_APPEND(self->impl->bufferList, buffer);
	++self->impl->bufferCount;

	return buffer;
}

void xprRenderTargetReleaseBuffer(XprRenderTarget* self, XprRenderBufferHandle buffer)
{
	if(nullptr == self)
		return;

	if(nullptr == buffer)
		return;

	((XprRenderBuffer*)buffer)->acquired = XprFalse;
}

struct XprTexture* XprRenderTarget_getTexture(XprRenderTarget* self, XprRenderBufferHandle buffer)
{
	if(nullptr == self)
		return nullptr;

	if(nullptr == buffer)
		return nullptr;

	return ((XprRenderBuffer*)buffer)->texture;
}

GLenum glAttachmentPoints[] =
{	GL_COLOR_ATTACHMENT0,
#if !defined(XPR_GLES_2)
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7,
#endif
};

void xprRenderTargetPreRender(XprRenderTarget* self, XprRenderBufferHandle* colors, XprRenderBufferHandle depth)
{
	size_t bufCnt;
	XprRenderBuffer** curr;
	if(nullptr == self) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, self->impl->glName);
	
	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = (XprRenderBuffer**)colors;
		while(*curr != nullptr) {
			XprTexture* tex = (*curr)->texture;
			glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachmentPoints[bufCnt], tex->impl->glTarget, tex->impl->glName, 0);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		XprTexture* tex = ((XprRenderBuffer*)depth)->texture;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->impl->glTarget, tex->impl->glName, 0);
	}

#if !defined(XPR_GLES_2)
	// assign buffer bindings
	glDrawBuffers(bufCnt, glAttachmentPoints);
#endif
	{	// check for framebuffer's complete status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(GL_FRAMEBUFFER_COMPLETE != status) {
			XprDbgStr("imcomplete framebuffer status:%x", status);
		}
	}
}

void xprRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax)
{
	glViewport((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
	glDepthRange(zmin, zmax);
}

void xprRenderTargetClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void xprRenderTargetClearDepth(float z)
{
	glClearDepth(z);
	glClear(GL_DEPTH_BUFFER_BIT);
}