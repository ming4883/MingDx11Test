#ifndef __XPRENDER_TEXTURE_D3D9_H__
#define __XPRENDER_TEXTURE_D3D9_H__

#include "API.d3d9.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTextureFormatMapping
{
	XprTextureFormat name;
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

#endif	// __XPRENDER_TEXTURE_D3D9_H__