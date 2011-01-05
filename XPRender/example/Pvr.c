#include "Pvr.h"

void Pvr_getTextureInfo(const PvrDataType* pvr, size_t* width, size_t* height, size_t* mipCount)
{
	if(nullptr == pvr)
		return;

	*height = (size_t)pvr[1];
	*width = (size_t)pvr[2];
	*mipCount = (size_t)pvr[3];
}

const void* Pvr_getTextureData(const PvrDataType* pvr, size_t* dataSizeInByte)
{
	if(nullptr == pvr)
		return nullptr;

	*dataSizeInByte = (size_t)pvr[5];

	return &pvr[13];
}

XprTexture* Pvr_createTexture(const PvrDataType* pvr)
{
	XprTexture* tex;
	size_t width, height, mipCount, dataSize;
	const void* data;
	if(nullptr == pvr)
		return nullptr;

	Pvr_getTextureInfo(pvr, &width, &height, &mipCount);
	data = Pvr_getTextureData(pvr, &dataSize);

	if(nullptr == pvr)
		return nullptr;

	tex = XprTexture_alloc();

	XprTexture_init(tex, width, height, mipCount, 1, "unormR8G8B8A8");

	if(dataSize == tex->surfSizeInByte)
		memcpy(tex->data, data, dataSize);

	XprTexture_commit(tex);

	return tex;
}