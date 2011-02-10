#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTexture;
typedef struct XprRenderBuffer;

typedef enum XprRenderTargetFlag
{
	XprRenderTargetFlag_Inited = 1 << 0,
} XprRenderTargetFlag;

typedef struct XprRenderTargetImpl;

typedef struct XprRenderTarget
{
	size_t flags;
	size_t width;
	size_t height;
	struct XprRenderTargetImpl* impl;

} XprRenderTarget;

typedef void* XprRenderBufferHandle;

XprRenderTarget* XprRenderTarget_alloc();

void XprRenderTarget_free(XprRenderTarget* self);

void XprRenderTarget_init(XprRenderTarget* self, size_t width, size_t height);

XprRenderBufferHandle XprRenderTarget_acquireBuffer(XprRenderTarget* self, const char* format);

void XprRenderTarget_releaseBuffer(XprRenderTarget* self, XprRenderBufferHandle buffer);

struct XprTexture* XprRenderTarget_getTexture(XprRenderTarget* self, XprRenderBufferHandle buffer);

void XprRenderTarget_preRender(XprRenderTarget* self, XprRenderBufferHandle* colors, XprRenderBufferHandle depth);

void XprRenderTarget_setViewport(float x, float y, float w, float h, float zmin, float zmax);

void XprRenderTarget_clearColor(float r, float g, float b, float a);

void XprRenderTarget_clearDepth(float z);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__