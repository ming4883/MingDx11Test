#include "RenderTarget.gl.h"
#include "Memory.h"

XprRenderTarget* xprRenderTargetAlloc()
{
	XprRenderTargetImpl* self = xprMemory()->alloc(sizeof(XprRenderTargetImpl), "XprRenderTarget");
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
			xprMemory()->free(it, "XprRenderTarget");
		}

		glDeleteFramebuffers(1, &impl->glName);
	}
	xprMemory()->free(self, "XprRenderTarget");
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

	glGenFramebuffers(1, &impl->glName);
	
	self->flags |= XprRenderTarget_Inited;
}

XprRenderBuffer* xprRenderTargetAcquireBuffer(XprRenderTarget* self, size_t width, size_t height, XprGpuFormat format)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	XprRenderBufferImpl* buffer;
	XprRenderBufferImpl* it;
	LL_FOREACH(impl->bufferList, it) {
		XprTexture* tex = it->i.texture;
		if(!it->acquired && (width == tex->width) && (height == tex->height) && (format == tex->format)) {
			return &it->i;
		}
	}

	buffer = xprMemory()->alloc(sizeof(XprRenderBufferImpl), "XprRenderTarget");
	buffer->acquired = XprTrue;
	buffer->i.texture = xprTextureAlloc();
	xprTextureInitRtt(buffer->i.texture, width, height, 0, 1, format);

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

static GLenum xprGL_ATTACHMENT_POINT[] =
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

void xprRenderTargetPreRender(XprRenderTarget* self, XprRenderBuffer** colors, XprRenderBuffer* depth)
{
	XprRenderTargetImpl* impl = (XprRenderTargetImpl*)self;

	size_t bufCnt;
	XprRenderBuffer** curr;
	if(nullptr == self) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, impl->glName);
	
	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = (XprRenderBuffer**)colors;
		while(*curr != nullptr) {
			XprTexture* tex = (*curr)->texture;
			glFramebufferTexture2D(GL_FRAMEBUFFER, xprGL_ATTACHMENT_POINT[bufCnt], ((XprTextureImpl*)tex)->glTarget, ((XprTextureImpl*)tex)->glName, 0);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		XprTexture* tex = depth->texture;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ((XprTextureImpl*)tex)->glTarget, ((XprTextureImpl*)tex)->glName, 0);
	}

#if !defined(XPR_GLES_2)
	// assign buffer bindings
	glDrawBuffers(bufCnt, xprGL_ATTACHMENT_POINT);
#endif
	{	// check for framebuffer's complete status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(GL_FRAMEBUFFER_COMPLETE != status) {
			xprDbgStr("imcomplete framebuffer status: %x\n", status);
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