#ifndef __XPRENDER_TEXTURE_GL3_H__
#define __XPRENDER_TEXTURE_GL3_H__

#include "Texture.h"
#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTextureImpl
{
	int glTarget;
	int glName;
} XprTextureImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__