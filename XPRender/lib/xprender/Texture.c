#include "Texture.gl3.h"
#include "StrUtil.h"

XprTextureFormatMapping XprTextureFormatMappings[] = {
	{"unormR8G8B8A8", 4, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV},
	{"floatR16", 2, GL_R16F, GL_RED, GL_HALF_FLOAT},
	{"floatR32", 4, GL_R32F, GL_RED, GL_FLOAT},
	{"floatR16G16B16A16", 8, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
	{"floatR32G32B32A32", 16, GL_RGBA32F, GL_RGBA, GL_FLOAT},
};

XprTextureFormatMapping* XprTextureFormatMapping_Get(const char* name)
{
	size_t i=0;
	for(i=0; i<XprCountOf(XprTextureFormatMappings); ++i) {
		XprTextureFormatMapping* mapping = &XprTextureFormatMappings[i];
		if(strcasecmp(name, mapping->name) == 0)
			return mapping;
	}

	return nullptr;
}

XprTexture* XprTexture_alloc()
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
			offset += self->impl->glFormatMapping->pixelSize * (*mipWidth) * (*mipHeight);
			if(*mipWidth > 1) *mipWidth /= 2;
			if(*mipHeight > 1) *mipHeight /= 2;
		}
	} while(++i < mipIndex);

	return offset;
}

void XprTexture_init(XprTexture* self, size_t width, size_t height, size_t mipLevels, size_t sliceCount, const char* format)
{
	if(self->flags & XprTextureFlag_Inited) {
		XprDbgStr("texture already inited!\n");
		return;
	}

	if(sliceCount > 1) {
		XprDbgStr("Current not support sliceCount > 1!\n");
		return;
	}

	self->impl->glFormatMapping = XprTextureFormatMapping_Get(format);
	
	if(nullptr == self->impl->glFormatMapping) {
		XprDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	strcpy(self->format, format);
	self->width = width;
	self->height = height;
	self->mipLevels = mipLevels;
	self->sliceCount = sliceCount;

	{
		size_t tmpw, tmph;
		self->sliceSizeInByte = XprTexture_getMipLevelOffset(self, self->mipLevels, &tmpw, &tmph);
		self->data = (unsigned char*)malloc(self->sliceSizeInByte * self->sliceCount);
		memset(self->data, 0, self->sliceSizeInByte * self->sliceCount);
	}

	glGenTextures(1, &self->impl->glName);
	
	if(self->sliceCount == 1) {
		self->impl->glTarget = GL_TEXTURE_2D;
	}

	XprTexture_commit(self);
}

unsigned char* XprTexture_getMipLevel(XprTexture* self, size_t sliceIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	if(nullptr == self)
		return nullptr;

	if(sliceIndex >= self->sliceCount)
		return nullptr;

	if(mipIndex > self->mipLevels)
		return nullptr;

	return self->data + sliceIndex * self->sliceSizeInByte + XprTexture_getMipLevelOffset(self, self->mipLevels, mipWidth, mipHeight);
}

void XprTexture_commit(XprTexture* self)
{
	size_t i;
	XprTextureFormatMapping* mapping;

	if(nullptr == self)
		return;

	if(nullptr == self->impl->glFormatMapping)
		return;
	
	mapping = self->impl->glFormatMapping;

	if(self->sliceCount == 1) {
		
		self->impl->glTarget = GL_TEXTURE_2D;

		glBindTexture(self->impl->glTarget, self->impl->glName);

		glTexImage2D(self->impl->glTarget, 0, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, self->data);
		for(i=1; i<=self->mipLevels; ++i) {
			size_t mipW, mipH;
			unsigned char* data = XprTexture_getMipLevel(self, 0, i, &mipW, &mipH);
			glTexImage2D(self->impl->glTarget, i, mapping->internalFormat, mipW, mipH, 0, mapping->format, mapping->type, data);
		}
	}
}

void XprTexture_free(XprTexture* self)
{
	if(nullptr == self)
		return;

	if(0 != self->impl->glName)
		glDeleteTextures(1, &self->impl->glName);

	if(nullptr != self->data)
		free(self->data);

	free(self);
}