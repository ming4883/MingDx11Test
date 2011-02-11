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

typedef enum XprGpuStateType
{
	// BlendFactor
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

	// PolygonMode
	XprGpuState_PolygonMode_Line,
	XprGpuState_PolygonMode_Fill,
} XprGpuStateType;

struct XprGpuStateImpl;

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

void XprGpuState_setCullEnabled(XprGpuState* self, XprBool value);

void XprGpuState_setBlendEnabled(XprGpuState* self, XprBool value);

void XprGpuState_setBlendFactorRGB(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest);

void XprGpuState_setBlendFactorA(XprGpuState* self, XprGpuStateType blendFactorSrc, XprGpuStateType blendFactorDest);

void XprGpuState_setPolygonMode(XprGpuState* self, XprGpuStateType polygonMode);


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_H__