#ifndef __XPRENDER_TEXTURE_H__
#define __XPRENDER_TEXTURE_H__

#include "Platform.h"

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
	char format[32];
	unsigned char* data;	// nullptr implies this is a render-target
	struct XprTextureImpl* impl;
} XprTexture;

XprTexture* XprTexture_alloc();

void XprTexture_free(XprTexture* self);

void XprTexture_init(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, const char* format);

void XprTexture_initRtt(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, const char* format);

unsigned char* XprTexture_getMipLevel(XprTexture* self, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight);

void XprTexture_commit(XprTexture* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__