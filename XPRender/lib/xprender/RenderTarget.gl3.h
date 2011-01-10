#ifndef __XPRENDER_RENDERTARGET_GL3_H__
#define __XPRENDER_RENDERTARGET_GL3_H__

#include "RenderTarget.h"

#include<GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprRenderTargetImpl
{
	int glName;
} XprRenderTargetImpl;


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_RENDERTARGET_GL3_H__