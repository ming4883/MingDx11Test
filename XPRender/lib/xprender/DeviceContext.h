#ifndef __XPRENDER_DEVICECONTEXT_H__
#define __XPRENDER_DEVICECONTEXT_H__

#include "Platform.h"
#include "StrHash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprDeviceContextFlag
{
	XprDeviceContextFlag_Inited = 1 << 0,
} XprDeviceContextFlag;

typedef struct XprDeviceContextImpl;

typedef struct XprDeviceContext
{
	size_t flags;	// combinations of XprDeviceContextFlag
	
	struct XprDeviceContextImpl* impl;

} XprDeviceContext;

XprDeviceContext* XprDeviceContext_alloc();

void XprDeviceContext_free(XprDeviceContext* self);

void XprDeviceContext_init(XprDeviceContext* self);

void XprDeviceContext_preRender(XprDeviceContext* self);

void XprDeviceContext_setBool(XprDeviceContext* self, XprHashCode state, XprBool value);


#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_DEVICECONTEXT_H__