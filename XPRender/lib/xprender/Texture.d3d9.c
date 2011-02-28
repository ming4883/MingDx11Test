#include "Texture.d3d9.h"
#include "StrUtil.h"

XprTextureFormatMapping XprTextureFormatMappings[] = {
	{XprTexture_UnormR8G8B8A8, 4, D3DFMT_A8R8G8B8},
	{XprTexture_UnormR8, 1, D3DFMT_A8},
	{XprTexture_FloatR16, 2, D3DFMT_R16F},
	{XprTexture_FloatR32, 4, D3DFMT_R32F},
	{XprTexture_FloatR16G16B16A16, 8, D3DFMT_A16B16G16R16F},
	{XprTexture_FloatR32G32B32A32, 16, D3DFMT_A32B32G32R32F},
};

XprTextureFormatMapping* XprTextureFormatMapping_Get(XprTextureFormat xprFormat)
{
	size_t i=0;
	for(i=0; i<XprCountOf(XprTextureFormatMappings); ++i) {
		XprTextureFormatMapping* mapping = &XprTextureFormatMappings[i];
		if(xprFormat == mapping->xprFormat)
			return mapping;
	}

	return nullptr;
}

XprTexture* xprTextureAlloc()
{
	XprTexture* self;
	XprAllocWithImpl(self, XprTexture, XprTextureImpl);
	return self;
}

size_t XprTexture_getMipLevelOffset(XprTexture* self, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	size_t i = 0;
	size_t offset = 0;
	
	*mipWidth = self->width;
	*mipHeight = self->height;
	
	do {
		if(i < mipIndex) {
			offset += self->impl->apiFormatMapping->pixelSize * (*mipWidth) * (*mipHeight);
			if(*mipWidth > 1) *mipWidth /= 2;
			if(*mipHeight > 1) *mipHeight /= 2;
		}
	} while(++i < mipIndex);

	return offset;
}

void xprTextureInit(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, XprTextureFormat format)
{
	if(self->flags & XprTextureFlag_Inited) {
		XprDbgStr("texture already inited!\n");
		return;
	}

	if(surfCount > 1) {
		XprDbgStr("Current not support surfCount > 1!\n");
		return;
	}

	self->impl->apiFormatMapping = XprTextureFormatMapping_Get(format);
	
	if(nullptr == self->impl->apiFormatMapping) {
		XprDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	// init cache memory
	{
		size_t tmpw, tmph;
		self->surfSizeInByte = XprTexture_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = (unsigned char*)malloc(self->surfSizeInByte * self->surfCount);
		memset(self->data, 0, self->surfSizeInByte * self->surfCount);
	}

	// create texture
	{
		HRESULT hr;
		hr = IDirect3DDevice9_CreateTexture(xprAPI.d3ddev, width, height, self->mipCount+1, 0, self->impl->apiFormatMapping->d3dFormat, D3DPOOL_MANAGED, &self->impl->d3dtex, nullptr);
		if(FAILED(hr)) {
			XprDbgStr("d3d9 failed to create texture %8x", hr);
			return;
		}
	}

	xprTextureCommit(self);
}

void xprTextureInitRtt(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, XprTextureFormat format)
{
	if(self->flags & XprTextureFlag_Inited) {
		XprDbgStr("texture already inited!\n");
		return;
	}

	if(surfCount > 1) {
		XprDbgStr("Current not support surfCount > 1!\n");
		return;
	}

	self->impl->apiFormatMapping = XprTextureFormatMapping_Get(format);
	
	if(nullptr == self->impl->apiFormatMapping) {
		XprDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	// set cache memory to null
	{
		size_t tmpw, tmph;
		self->surfSizeInByte = XprTexture_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = nullptr;
	}

	// create d3d texture
	{
		HRESULT hr;
		hr = IDirect3DDevice9_CreateTexture(xprAPI.d3ddev, width, height, self->mipCount+1, D3DUSAGE_RENDERTARGET, self->impl->apiFormatMapping->d3dFormat, D3DPOOL_MANAGED, &self->impl->d3dtex, nullptr);
		if(FAILED(hr)) {
			XprDbgStr("d3d9 failed to create texture %8x", hr);
			return;
		}
	}
}


unsigned char* xprTextureGetMipLevel(XprTexture* self, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	if(nullptr == self)
		return nullptr;

	if(surfIndex >= self->surfCount)
		return nullptr;

	if(mipIndex > self->mipCount)
		return nullptr;

	if(nullptr == self->data)
		return nullptr;

	return self->data + (surfIndex * self->surfSizeInByte) + XprTexture_getMipLevelOffset(self, mipIndex, mipWidth, mipHeight);
}

void xprTextureCommit(XprTexture* self)
{
	const XprTextureFormatMapping* mapping;

	if(nullptr == self)
		return;

	if(nullptr == self->impl->apiFormatMapping)
		return;
	
	mapping = self->impl->apiFormatMapping;

	if(self->surfCount == 1) {
		HRESULT hr;
		size_t i;

		for(i=1; i<=self->mipCount; ++i) {
			size_t mipW, mipH;
			D3DLOCKED_RECT locked;
			unsigned char* data = xprTextureGetMipLevel(self, 0, i, &mipW, &mipH);

			hr = IDirect3DTexture9_LockRect(self->impl->d3dtex, i, &locked, nullptr, D3DLOCK_DISCARD);
			if(FAILED(hr))
				continue;

			memcpy(locked.pBits, data, mipW * mipH * self->impl->apiFormatMapping->pixelSize);
			IDirect3DTexture9_UnlockRect(self->impl->d3dtex, i);
		}
	}
}

void xprTextureFree(XprTexture* self)
{
	if(nullptr == self)
		return;

	if(nullptr != self->data)
		free(self->data);

	if(nullptr != self->impl->d3dtex) {
		IDirect3DTexture9_Release(self->impl->d3dtex);
	}
	
	free(self);
}