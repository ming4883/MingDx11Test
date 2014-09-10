#ifndef __XPRENDER_GPUSTATE_GL_H__
#define __XPRENDER_GPUSTATE_GL_H__

#include "API.gl.h"
#include "GpuState.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprGpuStateImpl
{
	XprGpuState i;
	XprGpuStateDesc last;
	
} XprGpuStateImpl;

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_GL_H__