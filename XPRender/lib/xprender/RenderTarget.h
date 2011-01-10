#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTexture;

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

XprRenderTarget* XprRenderTarget_alloc();

void XprRenderTarget_free(XprRenderTarget* self);

void XprRenderTarget_init(XprRenderTarget* self, size_t width, size_t height);

XprHandle XprRenderTarget_acquireBuffer(XprRenderTarget* self, const char* format);

void XprRenderTarget_releaseBuffer(XprRenderTarget* self, XprHandle handle);

struct XprTexture* XprRenderTarget_getTexture(XprRenderTarget* self, XprHandle handle);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__