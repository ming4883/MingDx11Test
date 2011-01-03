#include "Texture.h"

#include <GL/glew.h>

#include <string.h>
#if defined(XPR_VC)
#define strcasecmp stricmp
#endif

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
	XprTexture* self = (XprTexture*)malloc(sizeof(XprTexture));
	memset(self, 0, sizeof(XprTexture));
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
	self->data = malloc(mapping->pixelSize * self->width * self->height * self->arraySize);

	glGenTextures(1, &self->name);

	if(arraySize == 1) {
		size_t i;
		self->target = GL_TEXTURE_2D;
		glTexImage2D(self->target, 0, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, nullptr);
		for(i=0; i<self->mipLevels; ++i)
			glTexImage2D(self->target, i, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, nullptr);
	}
}

void XprTexture_commit(XprTexture* self)
{
}

void XprTexture_free(XprTexture* self)
{
	if(nullptr == self)
		return;

	if(0 != self->name)
		glDeleteTextures(1, &self->name);

	if(nullptr != self->data)
		free(self->data);
}