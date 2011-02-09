#ifndef __XPRENDER_GPUSTATE_GL3_H__
#define __XPRENDER_GPUSTATE_GL3_H__

#include "GpuState.h"

#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuStateImpl
{
	XprBool depthTest;
	XprBool depthWrite;
	XprBool culling;
	XprBool blending;
	XprGpuStateType blendFactorSrc;
	XprGpuStateType blendFactorDest;
	XprGpuStateType blendFactorSrcAlpha;
	XprGpuStateType blendFactorDestAlpha;

	XprGpuStateType polygonMode;
	
} XprGpuStateImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_GL3_H__