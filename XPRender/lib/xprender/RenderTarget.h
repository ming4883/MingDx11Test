#ifndef __XPRENDER_RENDERTARGET_H__
#define __XPRENDER_RENDERTARGET_H__

#include "Platform.h"
#include "GpuFormat.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprTexture;
struct XprRenderBuffer;

typedef enum XprRenderTargetFlag
{
	XprRenderTarget_Inited = 1 << 0,
} XprRenderTargetFlag;

typedef struct XprRenderTarget
{
	size_t flags;
	size_t width;
	size_t height;

} XprRenderTarget;

typedef struct XprRenderBuffer
{
	struct XprTexture* texture;
} XprRenderBuffer;

XprRenderTarget* xprRenderTargetAlloc();

void xprRenderTargetFree(XprRenderTarget* self);

void xprRenderTargetInit(XprRenderTarget* self, size_t width, size_t height);

XprRenderBuffer* xprRenderTargetAcquireBuffer(XprRenderTarget* self, XprGpuFormat format);

void xprRenderTargetReleaseBuffer(XprRenderTarget* self, XprRenderBuffer* buffer);

void xprRenderTargetPreRender(XprRenderTarget* self, XprRenderBuffer** colors, XprRenderBuffer* depth);

void xprRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax);

void xprRenderTargetClearColor(float r, float g, float b, float a);

void xprRenderTargetClearDepth(float z);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_H__