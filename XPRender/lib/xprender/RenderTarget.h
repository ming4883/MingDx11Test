#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTexture;

typedef struct XprRenderTarget
{
	int name;
	size_t flags;
} XprRenderTarget;

XprRenderTarget* XprRenderTarget_request(size_t width, size_t height);

XprHandle XprRenderTarget_acquireBuffer(XprRenderTarget* self, const char* format);

void XprRenderTarget_releaseBuffer(XprRenderTarget* self, XprHandle handle);

struct XprTexture* XprRenderTarget_getTexture(XprRenderTarget* self, XprHandle handle);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__