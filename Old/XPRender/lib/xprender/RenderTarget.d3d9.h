#ifndef __XPRENDER_RENDERTARGET_D3D9_H__
#define __XPRENDER_RENDERTARGET_D3D9_H__

#include "API.d3d9.h"
#include "RenderTarget.h"
#include "Texture.d3d9.h"
#include "uthash/utlist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprRenderBufferImpl
{
	XprRenderBuffer i;

	struct XprRenderBufferImpl* next;
	struct XprRenderBufferImpl* last;
	XprBool acquired;
	IDirect3DSurface9* d3dsurf;

} XprRenderBufferImpl;

typedef struct XprRenderTargetImpl
{
	XprRenderTarget i;

	size_t bufferCount;
	struct XprRenderBufferImpl* bufferList;
} XprRenderTargetImpl;


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_D3D9_H__