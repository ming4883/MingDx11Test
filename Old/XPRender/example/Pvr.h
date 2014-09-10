#ifndef __EXAMPLE_PVR_H__
#define __EXAMPLE_PVR_H__

#include "../lib/xprender/Platform.h"
#include "../lib/xprender/Texture.h"

typedef unsigned long PvrDataType;

void Pvr_getTextureInfo(const PvrDataType* pvr, size_t* width, size_t* height, size_t* mipCount);

const void* Pvr_getTextureData(const PvrDataType* pvr, size_t* dataSizeInByte);

XprTexture* Pvr_createTexture(const PvrDataType* pvr);

#endif	// __EXAMPLE_PVR_H__