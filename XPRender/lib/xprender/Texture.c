#include "Texture.gl3.h"
#include "StrUtil.h"

typedef struct XprTextureFormatMapping
{
	const char* name;
	size_t pixelSize;
	int internalFormat;
	int format;
	int type;
} XprTextureFormatMapping;

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

void XprTexture_init(XprTexture* self, size_t width, size_t height, size_t mipLevels, size_t arraySize, const char* format)
{
	XprTextureFormatMapping* mapping = XprTextureFormatMapping_Get(format);

	if(self->flags & XprTextureFlag_Inited) {
		XprDbgStr("texture already inited!\n");
		return;
	}

	if(arraySize > 1) {
		XprDbgStr("Current not support arraySize > 1!\n");
		return;
	}
	
	if(nullptr == mapping) {
		XprDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	strcpy(self->format, format);
	self->width = width;
	self->height = height;
	self->mipLevels = mipLevels;
	self->arraySize = arraySize;

	{
		size_t tmpw, tmph;
		unsigned char* tmpPtr = XprTexture_getMipLevel(self, 0, mipLevels, &tmpw, &tmph);
		self->elementSizeInByte = (size_t)tmpPtr;
		self->data = (unsigned char*)malloc(self->elementSizeInByte);
	}

	glGenTextures(1, &self->impl->glName);
	
	if(arraySize == 1) {
		size_t i;
		self->impl->glTarget = GL_TEXTURE_2D;

		glBindTexture(self->impl->glTarget, self->impl->glName);

		glTexImage2D(self->impl->glTarget, 0, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, nullptr);
		for(i=0; i<self->mipLevels; ++i)
			glTexImage2D(self->impl->glTarget, i, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, nullptr);
	}
}

unsigned char* XprTexture_getMipLevel(XprTexture* self, size_t arrayIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	size_t i;
	size_t offset;
	XprTextureFormatMapping* mapping;

	if(nullptr == self)
		return nullptr;

	if(arrayIndex >= self->arraySize)
		return nullptr;

	if(mipIndex > self->mipLevels)
		return nullptr;

	mapping = XprTextureFormatMapping_Get(self->format);

	*mipWidth = self->width;
	*mipHeight = self->height;
	offset = 0;
	i = 0;

	do {
		if(i < mipIndex) {
			offset += mapping->pixelSize * (*mipWidth) * (*mipHeight);
			if(*mipWidth > 1) *mipWidth /= 2;
			if(*mipHeight > 1) *mipHeight /= 2;
		}
	} while(++i < mipIndex);

	return self->data + offset;
}

void XprTexture_commit(XprTexture* self)
{
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