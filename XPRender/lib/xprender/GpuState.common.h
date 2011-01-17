#ifndef __XPRENDER_GPUSTATE_COMMON_H__
#define __XPRENDER_GPUSTATE_COMMON_H__

#include "Platform.h"
#include "StrHash.h"

#define XprGpuState_DepthTestEnable XPR_HASH("DepthTestEnabled")
#define XprGpuState_DepthWriteEnable XPR_HASH("DepthWriteEnabled")
#define XprGpuState_CullingEnable XPR_HASH("CullingEnabled")

#endif	// __XPRENDER_GPUSTATE_COMMON_H__