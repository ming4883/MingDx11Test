#ifndef __XPRENDER_TEXTURE_D3D9_H__
#define __XPRENDER_TEXTURE_D3D9_H__

#include "API.d3d9.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTextureFormatMapping
{
	XprTextureFormat xprFormat;
	size_t pixelSize;
	D3DFORMAT d3dFormat;
} XprTextureFormatMapping;

typedef struct XprTextureImpl
{
	IDirect3DTexture9* d3dtex;
	struct XprTextureFormatMapping* apiFormatMapping;
} XprTextureImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_D3D9_H__