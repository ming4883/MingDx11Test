#ifndef __XPRENDER_GPUSTATE_H__
#define __XPRENDER_GPUSTATE_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprGpuStateFlag
{
	XprGpuState_Inited = 1 << 0,
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

typedef struct XprGpuStateDesc
{
	XprBool depthTest;
	XprBool depthWrite;
	XprBool cull;
	XprBool blend;
	XprGpuStateType blendFactorSrcRGB;
	XprGpuStateType blendFactorDestRGB;
	XprGpuStateType blendFactorSrcA;
	XprGpuStateType blendFactorDestA;
	XprGpuStateType polygonMode;
} XprGpuStateDesc;

typedef struct XprGpuState
{
	size_t flags;	// combinations of XprGpuStateFlag
	XprGpuStateDesc desc;

} XprGpuState;

XprGpuState* xprGpuStateAlloc();

void xprGpuStateFree(XprGpuState* self);

void xprGpuStateInit(XprGpuState* self);

void xprGpuStatePreRender(XprGpuState* self);


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_H__