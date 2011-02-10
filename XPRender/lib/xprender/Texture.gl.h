#ifndef __XPRENDER_TEXTURE_GL_H__
#define __XPRENDER_TEXTURE_GL_H__

#include "Opengl.h"
#include "Texture.h"
#include "Texture.common.h"

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