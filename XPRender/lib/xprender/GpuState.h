#ifndef __XPRENDER_GPUSTATE_H__
#define __XPRENDER_GPUSTATE_H__

#include "Platform.h"
#include "StrHash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprGpuStateFlag
{
	XprGpuStateFlag_Inited = 1 << 0,
} XprGpuStateFlag;

typedef struct XprGpuStateImpl;

typedef struct XprGpuState
{
	size_t flags;	// combinations of XprGpuStateFlag
	
	struct XprGpuStateImpl* impl;

} XprGpuState;

XprGpuState* XprGpuState_alloc();

void XprGpuState_free(XprGpuState* self);

void XprGpuState_init(XprGpuState* self);

void XprGpuState_preRender(XprGpuState* self);

void XprGpuState_setBool(XprGpuState* self, XprHashCode state, XprBool value);


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_GPUSTATE_H__