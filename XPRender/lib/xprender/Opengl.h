#ifndef __XPRENDER_OPENGL_H__
#define __XPRENDER_OPENGL_H__

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

#endif	// __XPRENDER_OPENGL_H__