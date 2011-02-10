#ifndef __XPRENDER_RENDERTARGET_GL_H__
#define __XPRENDER_RENDERTARGET_GL_H__

#include "Opengl.h"
#include "RenderTarget.h"
#include "Texture.gl.h"
#include "uthash/utlist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprRenderBuffer
{
	struct XprRenderBuffer* next;
	struct XprRenderBuffer* last;
	XprTexture* texture;
	XprBool acquired;
} XprRenderBuffer;

typedef struct XprRenderTargetImpl
{
	int glName;
	size_t bufferCount;
	struct XprRenderBuffer* bufferList;
} XprRenderTargetImpl;


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_GL_H__