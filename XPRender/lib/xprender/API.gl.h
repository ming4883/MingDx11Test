#ifndef __XPRENDER_API_GL_H__
#define __XPRENDER_API_GL_H__

#ifdef XPR_ANDROID
#	include <GLES2/gl2.h>
#	define XPR_GLES_2
#else
#	include <GL/glew.h>
#endif

#if defined(XPR_GLES_2)
#	define glClearDepth glClearDepthf
#	define glDepthRange glDepthRangef
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprAPI
{
	unsigned int gpuInputId;
} XprAPI;

extern XprAPI xprAPI;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_API_GL_H__