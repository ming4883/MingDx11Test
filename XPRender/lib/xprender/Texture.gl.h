#ifndef __XPRENDER_TEXTURE_GL_H__
#define __XPRENDER_TEXTURE_GL_H__

#include "API.gl.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTextureGpuFormatMapping
{
	XprGpuFormat xprFormat;
	size_t pixelSize;
	int internalFormat;
	int format;
	int type;
} XprTextureGpuFormatMapping;

typedef struct XprTextureImpl
{
	XprTexture i;
	int glTarget;
	int glName;
	struct XprTextureGpuFormatMapping* apiFormatMapping;
} XprTextureImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__