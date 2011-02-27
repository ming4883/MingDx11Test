#ifndef __XPRENDER_TEXTURE_H__
#define __XPRENDER_TEXTURE_H__

#include "Platform.h"
#include "Texture.format.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprTextureFlag
{
	XprTextureFlag_Inited = 1 << 0,
	XprTextureFlag_Dirty = 1 << 1,
} XprTextureFlag;

struct XprTextureImpl;

typedef struct XprTexture
{
	size_t flags;
	size_t width;
	size_t height;
	size_t mipCount;
	size_t surfCount;
	size_t surfSizeInByte;
	XprTextureFormat format;
	unsigned char* data;	// nullptr implies this is a render-target
	struct XprTextureImpl* impl;
} XprTexture;

XprTexture* xprTextureAlloc();

void xprTextureFree(XprTexture* self);

void xprTextureInit(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, XprTextureFormat format);

void xprTextureInitRtt(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, XprTextureFormat format);

unsigned char* xprTextureGetMipLevel(XprTexture* self, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight);

void xprTextureCommit(XprTexture* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__