#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xprRenderTarget
{
	int name;
	size_t flags;
} xprRenderTarget;

xprRenderTarget* xprRenderTarget_new();

void xprRenderTarget_free(xprRenderTarget* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__