#include "Texture.gl.h"
#include "StrUtil.h"

#if defined(XPR_GLES_2)
XprTextureGpuFormatMapping XprTextureGpuFormatMappings[] = {
	{XprGpuFormat_UnormR8G8B8A8, 4, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
	{XprGpuFormat_UnormR8, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
};
#else
XprTextureGpuFormatMapping XprTextureGpuFormatMappings[] = {
	{XprGpuFormat_UnormR8G8B8A8, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV},
	{XprGpuFormat_UnormR8, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE},
	{XprGpuFormat_FloatR16, 2, GL_R16F, GL_RED, GL_HALF_FLOAT},
	{XprGpuFormat_FloatR32, 4, GL_R32F, GL_RED, GL_FLOAT},
	{XprGpuFormat_FloatR16G16B16A16, 8, GL_RGBA16F, GL_BGRA, GL_HALF_FLOAT},
	{XprGpuFormat_FloatR32G32B32A32, 16, GL_RGBA32F, GL_BGRA, GL_FLOAT},
	{XprGpuFormat_Depth16, 2, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT},  
	{XprGpuFormat_Depth32, 4, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT},
};
#endif

XprTextureGpuFormatMapping* xprTextureGpuFormatMappingGet(XprGpuFormat xprFormat)
{
	size_t i=0;
	for(i=0; i<xprCountOf(XprTextureGpuFormatMappings); ++i) {
		XprTextureGpuFormatMapping* mapping = &XprTextureGpuFormatMappings[i];
		if(xprFormat == mapping->xprFormat)
			return mapping;
	}

	return nullptr;
}

XprTexture* xprTextureAlloc()
{
	XprTextureImpl* self = malloc(sizeof(XprTextureImpl));
	memset(self, 0, sizeof(XprTextureImpl));
	return &self->i;
}

size_t XprGpuFormat_getMipLevelOffset(XprTexture* self, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	size_t i = 0;
	size_t offset = 0;
	XprTextureImpl* impl = (XprTextureImpl*)self;
	
	*mipWidth = self->width;
	*mipHeight = self->height;
	
	do {
		if(i < mipIndex) {
			offset += impl->apiFormatMapping->pixelSize * (*mipWidth) * (*mipHeight);
			if(*mipWidth > 1) *mipWidth /= 2;
			if(*mipHeight > 1) *mipHeight /= 2;
		}
	} while(++i < mipIndex);

	return offset;
}

void xprTextureInit(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, XprGpuFormat format)
{
	XprTextureImpl* impl = (XprTextureImpl*)self;
	if(self->flags & XprTextureFlag_Inited) {
		xprDbgStr("texture already inited!\n");
		return;
	}

	if(surfCount > 1) {
		xprDbgStr("Current not support surfCount > 1!\n");
		return;
	}

	impl->apiFormatMapping = xprTextureGpuFormatMappingGet(format);
	
	if(nullptr == impl->apiFormatMapping) {
		xprDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = XprGpuFormat_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = (unsigned char*)malloc(self->surfSizeInByte * self->surfCount);
		memset(self->data, 0, self->surfSizeInByte * self->surfCount);
	}

	glGenTextures(1, &impl->glName);
	
	if(self->surfCount == 1) {
		impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(impl->glTarget, impl->glName);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	xprTextureCommit(self);
}

void xprTextureInitRtt(XprTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, XprGpuFormat format)
{
	XprTextureImpl* impl = (XprTextureImpl*)self;

	if(self->flags & XprTextureFlag_Inited) {
		xprDbgStr("texture already inited!\n");
		return;
	}

	if(surfCount > 1) {
		xprDbgStr("Current not support surfCount > 1!\n");
		return;
	}

	impl->apiFormatMapping = xprTextureGpuFormatMappingGet(format);
	
	if(nullptr == impl->apiFormatMapping) {
		xprDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = XprGpuFormat_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = nullptr;
	}

	glGenTextures(1, &impl->glName);
	
	if(self->surfCount == 1) {
		impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(impl->glTarget, impl->glName);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	//if(nullptr == self->data)
	//	return nullptr;

	return self->data + (surfIndex * self->surfSizeInByte) + XprGpuFormat_getMipLevelOffset(self, mipIndex, mipWidth, mipHeight);
}

void xprTextureCommit(XprTexture* self)
{
	const XprTextureGpuFormatMapping* mapping;
	XprTextureImpl* impl = (XprTextureImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr == impl->apiFormatMapping)
		return;
	
	mapping = impl->apiFormatMapping;

	if(self->surfCount == 1) {
		
		size_t i;

		glBindTexture(impl->glTarget, impl->glName);
		//glTexImage2D(impl->glTarget, 0, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, self->data);
		
		for(i=0; i<self->mipCount+1; ++i) {
			size_t mipW, mipH;
			unsigned char* data = xprTextureGetMipLevel(self, 0, i, &mipW, &mipH);
			glTexImage2D(impl->glTarget, i, mapping->internalFormat, mipW, mipH, 0, mapping->format, mapping->type, data);
		}
	}

	{
		GLenum err = glGetError();

		if(GL_NO_ERROR != err)
			xprDbgStr("failed to commit texture: 0x%04x\n", (int)err);
	}
}

void xprTextureFree(XprTexture* self)
{
	XprTextureImpl* impl = (XprTextureImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != self->data)
		free(self->data);

	if(0 != impl->glName)
		glDeleteTextures(1, &impl->glName);
	
	free(self);
}