#ifndef __XPRENDER_BUFFER_GL3_H__
#define __XPRENDER_BUFFER_GL3_H__

#include "Buffer.h"

#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprBufferImpl
{
	int glName;
	
} XprBufferImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_BUFFER_GL3_H__