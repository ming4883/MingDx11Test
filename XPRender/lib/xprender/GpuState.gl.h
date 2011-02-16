#ifndef __XPRENDER_GPUSTATE_GL_H__
#define __XPRENDER_GPUSTATE_GL_H__

#include "Opengl.h"
#include "GpuState.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuStateImpl
{
	/*
	XprBool depthTest;
	XprBool depthWrite;
	XprBool culling;
	XprBool blending;
	XprGpuStateType blendFactorSrc;
	XprGpuStateType blendFactorDest;
	XprGpuStateType blendFactorSrcAlpha;
	XprGpuStateType blendFactorDestAlpha;
	XprGpuStateType polygonMode;
	*/
	XprGpuStateDesc last;
	
} XprGpuStateImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_GL_H__