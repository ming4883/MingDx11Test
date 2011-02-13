#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprTexture;
struct XprRenderBuffer;

typedef enum XprRenderTargetFlag
{
	XprRenderTargetFlag_Inited = 1 << 0,
} XprRenderTargetFlag;

struct XprRenderTargetImpl;

typedef struct XprRenderTarget
{
	size_t flags;
	size_t width;
	size_t height;
	struct XprRenderTargetImpl* impl;

} XprRenderTarget;

typedef void* XprRenderBufferHandle;

XprRenderTarget* xprRenderTargetAlloc();

void xprRenderTargetFree(XprRenderTarget* self);

void xprRenderTargetInit(XprRenderTarget* self, size_t width, size_t height);

XprRenderBufferHandle xprRenderTargetAcquireBuffer(XprRenderTarget* self, const char* format);

void xprRenderTargetReleaseBuffer(XprRenderTarget* self, XprRenderBufferHandle buffer);

struct XprTexture* XprRenderTarget_getTexture(XprRenderTarget* self, XprRenderBufferHandle buffer);

void xprRenderTargetPreRender(XprRenderTarget* self, XprRenderBufferHandle* colors, XprRenderBufferHandle depth);

void xprRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax);

void xprRenderTargetClearColor(float r, float g, float b, float a);

void xprRenderTargetClearDepth(float z);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__