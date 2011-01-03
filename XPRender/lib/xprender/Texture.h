#ifndef __XPRENDER_TEXTURE_H__
#define __XPRENDER_TEXTURE_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprTexture
{
	size_t flags;
	size_t width;
	size_t height;
	size_t mipLevels;
	size_t arraySize;
	char format[32];
	unsigned char* data;
	int name;
	int target;
} XprTexture;

typedef enum XprTextureFlag
{
	XprTextureFlag_Inited = 1 << 0,
	XprTextureFlag_Dirty = 1 << 1,
} XprTextureFlag;

XprTexture* XprTexture_alloc();

void XprTexture_init(XprTexture* self, size_t width, size_t height, size_t mipLevels, size_t arraySize, const char* format);

void XprTexture_commit(XprTexture* self);

void XprTexture_free(XprTexture* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_TEXTURE_H__