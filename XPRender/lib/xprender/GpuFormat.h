#ifndef __XPRENDER_GPUFORMAT_H__
#define __XPRENDER_GPUFORMAT_H__

typedef enum XprGpuFormat
{
	XprGpuFormat_UnormR8G8B8A8,
	XprGpuFormat_UnormR8,
	XprGpuFormat_FloatR16,
	XprGpuFormat_FloatR32,
	XprGpuFormat_FloatR32G32,
	XprGpuFormat_FloatR32G32B32,
	XprGpuFormat_FloatR16G16B16A16,
	XprGpuFormat_FloatR32G32B32A32,
	XprGpuFormat_Depth16,
	XprGpuFormat_Depth32,

} XprGpuFormat;

#endif	// __XPRENDER_GPUFORMAT_H__