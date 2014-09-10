#ifndef __XPRENDER_RENDERTARGET_GL_H__
#define __XPRENDER_RENDERTARGET_GL_H__

#include "API.gl.h"
#include "RenderTarget.h"
#include "Texture.gl.h"
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

} XprRenderBufferImpl;

typedef struct XprRenderTargetImpl
{
	XprRenderTarget i;

	GLuint glName;
	size_t bufferCount;
	struct XprRenderBufferImpl* bufferList;
} XprRenderTargetImpl;


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_GL_H__
