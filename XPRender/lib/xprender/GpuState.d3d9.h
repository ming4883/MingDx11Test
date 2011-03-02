#ifndef __XPRENDER_GPUSTATE_D3D9_H__
#define __XPRENDER_GPUSTATE_D3D9_H__

#include "API.d3d9.h"
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

#endif	// __XPRENDER_GPUSTATE_D3D9_H__