#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprRenderTarget
{
	int name;
	size_t flags;
} XprRenderTarget;

XprRenderTarget* XprRenderTarget_new();

void XprRenderTarget_free(XprRenderTarget* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__