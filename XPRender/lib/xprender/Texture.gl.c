#include "Texture.gl.h"
#include "StrUtil.h"

#if defined(XPR_GLES_2)
XprTextureFormatMapping XprTextureFormatMappings[] = {
	{XprTexture_UnormR8G8B8A8, 4, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
	{XprTexture_UnormR8, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
};
#else
XprTextureFormatMapping XprTextureFormatMappings[] = {
	{XprTexture_UnormR8G8B8A8, 4, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV},
	{XprTexture_UnormR8, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE},
	{XprTexture_FloatR16, 2, GL_R16F, GL_RED, GL_HALF_FLOAT},
	{XprTexture_FloatR32, 4, GL_R32F, GL_RED, GL_FLOAT},
	{XprTexture_FloatR16G16B16A16, 8, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
	{XprTexture_FloatR32G32B32A32, 16, GL_RGBA32F, GL_RGBA, GL_FLOAT},
	{XprTexture_Depth16, 2, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT},  
	{XprTexture_Depth32, 4, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT},
};
#endif

XprTextureFormatMapping* XprTextureFormatMapping_Get(XprTextureFormat name)
{
	size_t i=0;
	for(i=0; i<XprCountOf(XprTextureFormatMappings); ++i) {
		XprTextureFormatMapping* mapping = &XprTextureFormatMappings[i];
		if(name == mapping->name)
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

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = XprTexture_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = (unsigned char*)malloc(self->surfSizeInByte * self->surfCount);
		memset(self->data, 0, self->surfSizeInByte * self->surfCount);
	}

	glGenTextures(1, &self->impl->glName);
	
	if(self->surfCount == 1) {
		self->impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(self->impl->glTarget, self->impl->glName);
		glTexParameteri(self->impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(self->impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = XprTexture_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = nullptr;
	}

	glGenTextures(1, &self->impl->glName);
	
	if(self->surfCount == 1) {
		self->impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(self->impl->glTarget, self->impl->glName);
		glTexParameteri(self->impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(self->impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	xprTextureCommit(self);
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
	size_t i;
	const XprTextureFormatMapping* mapping;

	if(nullptr == self)
		return;

	if(nullptr == self->impl->apiFormatMapping)
		return;
	
	mapping = self->impl->apiFormatMapping;

	if(self->surfCount == 1) {
		
		glBindTexture(self->impl->glTarget, self->impl->glName);

		glTexImage2D(self->impl->glTarget, 0, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, self->data);
		
		for(i=1; i<=self->mipCount; ++i) {
			size_t mipW, mipH;
			unsigned char* data = xprTextureGetMipLevel(self, 0, i, &mipW, &mipH);
			glTexImage2D(self->impl->glTarget, i, mapping->internalFormat, mipW, mipH, 0, mapping->format, mapping->type, data);
		}
	}
}

void xprTextureFree(XprTexture* self)
{
	if(nullptr == self)
		return;

	if(nullptr != self->data)
		free(self->data);

	if(0 != self->impl->glName)
		glDeleteTextures(1, &self->impl->glName);
	
	free(self);
}