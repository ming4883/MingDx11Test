#ifndef __XPRENDER_GPUSTATE_H__
#define __XPRENDER_GPUSTATE_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprGpuStateFlag
{
	XprGpuStateFlag_Inited = 1 << 0,
} XprGpuStateFlag;

typedef struct XprGpuStateImpl;

typedef enum XprGpuStateType
{
	XprGpuState_BlendFactor_One,
	XprGpuState_BlendFactor_Zero,
	XprGpuState_BlendFactor_SrcColor,
	XprGpuState_BlendFactor_OneMinusSrcColor,
	XprGpuState_BlendFactor_DestColor,
	XprGpuState_BlendFactor_OneMinusDestColor,
	XprGpuState_BlendFactor_SrcAlpha,
	XprGpuState_BlendFactor_OneMinusSrcAlpha,
	XprGpuState_BlendFactor_DestAlpha,
	XprGpuState_BlendFactor_OneMinusDestAlpha,
} XprGpuStateType;

typedef struct XprGpuState
{
	size_t flags;	// combinations of XprGpuStateFlag
	
	struct XprGpuStateImpl* impl;

} XprGpuState;

XprGpuState* XprGpuState_alloc();

void XprGpuState_free(XprGpuState* self);

void XprGpuState_init(XprGpuState* self);

void XprGpuState_preRender(XprGpuState* self);

void XprGpuState_setDepthTestEnabled(XprGpuState* self, XprBool value);

void XprGpuState_setDepthWriteEnabled(XprGpuState* self, XprBool value);

void XprGpuState_setCullingEnabled(XprGpuState* self, XprBool value);

void XprGpuState_setBlendingEnabled(XprGpuState* self, XprBool value);

void XprGpuState_setBlendFactors(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest);

void XprGpuState_setBlendAlphaFactors(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest);


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_H__