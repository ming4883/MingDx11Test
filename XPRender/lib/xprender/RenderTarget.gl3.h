#ifndef __XPRENDER_RENDERTARGET_GL3_H__
#define __XPRENDER_RENDERTARGET_GL3_H__

#include "RenderTarget.h"
#include "Texture.gl3.h"
#include "uthash/utlist.h"

#include <GL/glew.h>

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

#endif	// __XPRENDER_RENDERTARGET_GL3_H__