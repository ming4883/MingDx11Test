#ifndef __XPRENDER_API_GL_H__
#define __XPRENDER_API_GL_H__

#ifdef XPR_ANDROID
#	include <GLES2/gl2.h>
#	include <GLES2/glext.h>
#	define XPR_GLES_2

#elif defined(XPR_APPLE_IOS)
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
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
#if defined(XPR_APPLE_IOS)
	GLuint defFBOName;
	GLuint defColorBufName;
	GLuint defDepthBufName;
#endif
} XprAPI;

extern XprAPI xprAPI;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_API_GL_H__
