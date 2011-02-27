#ifndef __XPRENDER_TEXTURE_FORMAT_H__
#define __XPRENDER_TEXTURE_FORMAT_H__

typedef enum XprTextureFormat
{
	XprTexture_UnormR8G8B8A8,
	XprTexture_UnormR8,
	XprTexture_FloatR16,
	XprTexture_FloatR32,
	XprTexture_FloatR16G16B16A16,
	XprTexture_FloatR32G32B32A32,
	XprTexture_Depth16,
	XprTexture_Depth32,

} XprTextureFormat;

#endif	// __XPRENDER_TEXTURE_FORMAT_H__