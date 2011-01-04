#ifndef __XPRENDER_TEXTURE_GL3_H__
#define __XPRENDER_TEXTURE_GL3_H__

#include "Texture.h"
#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct XprTextureFormatMapping
{
	const char* name;
	size_t pixelSize;
	int internalFormat;
	int format;
	int type;
} XprTextureFormatMapping;

typedef struct XprTextureImpl
{
	int glTarget;
	int glName;
	struct XprTextureFormatMapping* glFormatMapping;
} XprTextureImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__